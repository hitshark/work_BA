/*
------------------------------------------------------------------------
--
-- File :                       transmit_ca.cpp
-- Author:                      Tao Li
-- Date :                       $Date: 2014/3/17 9:39:45 $
-- Version      :               $Revision: 1.0 $
-- Abstract     :               transaction level cycle current model
--                              of ahb interface
--                              This file describes the ahb interface in
--                              cycle accurate level using the TLM1.0 standard.
--
-- Modification History:
-- Date                 By      Version     Change Description
------------------------------------------------------------------------
*/

#include "transmit_ca.h"                         // Our header

using namespace sc_core;
using std::cout;
using std::endl;

// **********************************************************************
// constructor
// **********************************************************************

transmit_ca::transmit_ca                            // constructor
( sc_module_name name                               // module name
, const unsigned int ID                             // initiator ID
)
: sc_module           (name)                        // init module name
, m_ID                (ID)                          // init initiator ID
{
    // register thread process
    SC_THREAD(initiator_thread);
    SC_THREAD(ca_transmit_thread);
}

// **********************************************************************
// This thread takes generic payloads (gp) from the request FIFO that connects
// to the traffic package. When the transaction completes the
// gp is placed in the response FIFO to return it to the traffic generator.
// **********************************************************************

void transmit_ca::initiator_thread()   // initiator thread
{
	rd_req=false;
    wr_req=false;
    trans_status=true;
    tlm::tlm_generic_payload *transaction_ptr;    // transaction pointer
	sc_core::wait(5,sc_core::SC_NS);
    while (true)
    {
        // Read FIFO to Get new transaction GP from the traffic generator
        while(request_in_port->num_available()>=0)
        {
            transaction_ptr = request_in_port->read();  // get request from input fifo
            // extract the information from generic payload
            // including address,data length,streaming width,transaction type etc.
            sc_dt::uint64 addr_temp=transaction_ptr->get_address();

            //unsigned char *byte_enable=transaction_ptr->get_byte_enable_ptr();
            unsigned int trans_size=transaction_ptr->get_byte_enable_length();

            unsigned char *data_temp=transaction_ptr->get_data_ptr();
            unsigned int data_length=transaction_ptr->get_data_length();
            unsigned int stream_width=transaction_ptr->get_streaming_width();

            // store the information extracted from the generic payload
            // t_size is based on the byte enable length
            // t_burst is based on the data length and streaming width
            // if streaming width>0,addresses need to be wrapped
            t_addr=unsigned int(addr_temp);         // store address
            temp_addr=t_addr;
            switch(trans_size)                      // t_size
            {
                case 1:
                    t_size=BYTE;
					beat_count=data_length;
                    break;
                case 2:
                    t_size=HWORD;
					beat_count=data_length/2;
                    break;
                case 4:
                    t_size=WORD;
					beat_count=data_length/4;
                    break;
                default:
                    t_size=WORD;
					beat_count=0;
					break;
            }
            temp_beat=beat_count;                   // for the transfer of data between buffer and dynamic memory
			temp_beat_for_retry=beat_count;         // for retry and split transmit

            if(stream_width != 0)                   // wrap the address,set busrt type
            {
                if(stream_width != data_length)     // when data length is not equal to streaming width
                                                    // set response status to burst error
                                                    // then write back the transaction pointer to response fifo
                {
                    transaction_ptr->set_response_status(tlm::TLM_BURST_ERROR_RESPONSE);
                    response_out_port->write(transaction_ptr);
                    continue;
                }
                else
                {
                    switch(beat_count)
                    {
                        case 1:
                            t_burst=SINGLE;
                            break;
                        case 4:
                            t_burst=WRAP4;
                            break;
                        case 8:
                            t_burst=WRAP8;
                            break;
                        case 16:
                            t_burst=WRAP16;
                            break;
                        default:                    // default case,set burst type to INCR
                            t_burst=INCR;
							break;
                    }
                } // end if(stream_width == data_length)
            } // end if(stream_width != 0)
            else                                    // busrt type is increase,set the burst type
            {                                       // according to the number of beats
                switch(beat_count)
                {
                    case 1:
                        t_burst=SINGLE;
                        break;
                    case 4:
                        t_burst=INCR4;
                        break;
                    case 8:
                        t_burst=INCR8;
                        break;
                    case 16:
                        t_burst=INCR16;
                        break;
                    default:
                        t_burst=INCR;
						break;
                }
            } // end if(stream_width == 0)

            // check address to find out if address is aligned with
            // the transfer size,and if INCR4/8/16, it may cross 1KB boundary
            // WRAP4/8/16 transfers won't cross 1KB boundary.
            if(!check_addr(t_addr,t_burst,t_size))
            {
                transaction_ptr->set_response_status(tlm::TLM_ADDRESS_ERROR_RESPONSE);
                response_out_port->write(transaction_ptr);
                continue;
            }
            // set transaction type
            if(transaction_ptr->get_command()==tlm::TLM_READ_COMMAND)
            {
                t_write=READ;
                rd_req=TRUE;
            }
            else if(transaction_ptr->get_command()==tlm::TLM_WRITE_COMMAND)
            {
                t_write=WRITE;
                wr_req=TRUE;
            }
            else  // get_command() return tlm::TLM_IGNORE_COMMAND,set default case READ
                t_write=READ;

            if(t_write==WRITE)
            // when transaction type is write,put the write data to buffer
            // the START ADDRESS corresponding to t_wdata[beat_count-1]
            {
                while(temp_beat>0)
                {
                    if(t_size==WORD)
					{
                        t_wdata[temp_beat-1]=*reinterpret_cast<unsigned int *>(data_temp);
						data_temp=data_temp+4;
					}
                    else if(t_size==HWORD)
					{
						unsigned short int temp;
                        temp=*reinterpret_cast<unsigned short int *>(data_temp);
						t_wdata[temp_beat-1]=(unsigned int)temp;
						data_temp=data_temp+2;
					}
                    else if(t_size==BYTE)
					{
						unsigned char temp;
						temp=*data_temp;
                        t_wdata[temp_beat-1]=(unsigned int)temp;
						data_temp++;
					}
                    temp_beat--;
                }
            }

            // wait for the transaction complete
            // if read,handle the received data,put it to t_rdata buffer
            // set the response status
            // write the generic payload to response fifo
            wait(trans_complete);
            if(trans_status==TRUE)
            {
                if(t_write==READ)
                {
                    while(temp_beat>0)
                    {
                        if(t_size==WORD)
						{
                            *reinterpret_cast<unsigned int *>(data_temp)=t_rdata[temp_beat-1];
							data_temp=data_temp+4;
						}
                        else if(t_size==HWORD)
						{
                            *reinterpret_cast<unsigned short int *>(data_temp)=(unsigned short int)t_rdata[temp_beat-1];
							data_temp=data_temp+2;
						}
                        else if(t_size==BYTE)
						{
                            *data_temp=(unsigned char)t_rdata[temp_beat-1];
							data_temp++;
						}
                        temp_beat--;
                    }
                }
                transaction_ptr->set_response_status(tlm::TLM_OK_RESPONSE);
            }
            else
                transaction_ptr->set_response_status(tlm::TLM_GENERIC_ERROR_RESPONSE);
            response_out_port->write(transaction_ptr);    // send GP to output rsp fifo
        } // end while(request_in_port->num_available()>=0)
        sc_core::wait(5,SC_NS);
    } // end while true
} // end initiator_thread

void transmit_ca::ca_transmit_thread()
{
    trans_flag=false;
    trans_status=true;
	t_data=0;
    resp_state=OKAY;
    ahb_master_state=AHBM_BREQ;
    sc_core::wait(5,SC_NS);
    while(true)
    {
        wait(5,SC_NS);
        if(rd_req || wr_req)
        // if read or write transaction occurs
        // then start the bus request process
        {
            while(true)
            {
                switch(ahb_master_state)
                {
                    case AHBM_BREQ:
                    // interface method call
                    // using bus_req() method to start bus request
                    // if granted,advanced to the next state and
                    // start the first address phase
                    {
                        while(!(trans_port->bus_req()))
                            wait(5,SC_NS);
                        if(t_write==WRITE)
                        {
                            trans_port->nonseq_write(temp_addr);    // first address phase
                            ahb_master_state=AHBM_WRITE;
                        }
                        else
                        {
                            trans_port->nonseq_read(temp_addr);
                            ahb_master_state=AHBM_READ;
                        }
                        break;
                    }
                    case AHBM_READ:
                    // if transmit is single or the last data phase
                    // call last_read method to complete the transaction
                    // else use seq_read() method to send out address and
                    // data in each cycle
                    // then if read ok,put the read data back to buffer
                    // else if read error,change state to bus_request
                    // else if retry and split,restart the transaction
                    {
                        if(beat_count==1)
                        {
                            resp_state=trans_port->last_read(t_size,t_data);
                            if(resp_state==OKAY)
                            {
                                t_rdata[beat_count-1]=t_data; //handle the read data
                                beat_count--;
                                trans_status=true;              // set the transmit state
                                trans_flag=true;                // flag used for jumping out the while recycle
                                trans_complete.notify();        // event notification
                                                                // indicate that transaction is over
                                rd_req=false;                   // set read request to false
                                ahb_master_state=AHBM_BREQ;
                            }
                            else if(resp_state==EROR)
                            {
                                trans_status=false;             // set the transmit state
                                trans_flag=true;                // flag used for jumping out the while recycle
                                trans_complete.notify();        // event notification
                                                                // indicate that transaction is over
                                rd_req=false;                   // set read request to false
                                ahb_master_state=AHBM_BREQ;
                            }
                            else
                            {
                                ahb_master_state=AHBM_BREQ;
                                temp_addr=t_addr;               // retrieve the start address
                                beat_count=temp_beat_for_retry; // retrieve beat count
                            }
                        }
                        else
                        {
                            temp_addr=unsigned int (update_addr(temp_addr,t_burst,t_size));
                            resp_state=trans_port->seq_read(temp_addr,t_size,t_data);
                            if(resp_state==OKAY)
                            {
                                t_rdata[beat_count-1]=t_data; //handle the read data
                                beat_count--;
                            }
                            else if(resp_state==EROR)
                            {
                                trans_status=false;             // set the transmit state
                                trans_flag=true;                // flag used for jumping out the while recycle
                                trans_complete.notify();        // event notification
                                                                // indicate that transaction is over
                                rd_req=false;                   // set read request to false
                                ahb_master_state=AHBM_BREQ;
                            }
                            else
                            {
                                ahb_master_state=AHBM_BREQ;
                                temp_addr=t_addr;               // retrieve the start address
                                beat_count=temp_beat_for_retry; // retrieve beat count
                            }
                        }
                        break;
                    }
                    case AHBM_WRITE:
                    {
                        t_data=t_wdata[beat_count-1];       // send write data from write data buffer
                        if(beat_count==1)
                        {
                            resp_state=trans_port->last_write(t_size,t_data);
                            if(resp_state==OKAY)
                            {
                                beat_count--;
                                trans_status=true;          // set the transmit state
                                trans_flag=true;            // flag used for jumping out the while recycle
                                trans_complete.notify();    // event notification
                                                            // indicate that transaction is over
                                wr_req=false;               // set write request to false
                                ahb_master_state=AHBM_BREQ;
                            }
                            else if(resp_state==EROR)
                            {
                                trans_status=false;         // set the transmit state
                                trans_flag=true;            // flag used for jumping out the while recycle
                                trans_complete.notify();    // event notification
                                                            // indicate that transaction is over
                                wr_req=false;               // set write request to false
                                ahb_master_state=AHBM_BREQ;
                            }
                            else
                            {
                                ahb_master_state=AHBM_BREQ;
                                temp_addr=t_addr;               // retrieve the start address
                                beat_count=temp_beat_for_retry; // retrieve beat count
                            }
                        }
                        else
                        {
                            temp_addr=unsigned int (update_addr(temp_addr,t_burst,t_size));
                            resp_state=trans_port->seq_write(temp_addr,t_size,t_data);
                            if(resp_state==OKAY)
                            {
                                beat_count--;
                            }
                            else if(resp_state==EROR)
                            {
                                trans_status=false;             // set the transmit state
                                trans_flag=true;                // flag used for jumping out the while recycle
                                trans_complete.notify();        // event notification
                                                                // indicate that transaction is over
                                wr_req=false;                   // set write request to false
                                ahb_master_state=AHBM_BREQ;
                            }
                            else
                            {
                                ahb_master_state=AHBM_BREQ;
                                temp_addr=t_addr;               // retrieve the start address
                                beat_count=temp_beat_for_retry; // retrieve beat count
                            }
                        }
                        break;
                    }
                    default:;
                }
                if(trans_flag)
                {
                    trans_flag=false;
                    break;
                }
            }
        }
    }
}

// **********************************************************************
//   This function is aimed to find out if address is aligned with
//   the transfer size,and if INCR4/8/16, it may cross 1KB boundary
//	 WRAP4/8/16 transfers won't cross 1KB boundary.
// **********************************************************************

bool transmit_ca::check_addr(sc_uint<BUSWIDTH> address,
                  int burst,int size)
{
    // check if address is aligned with the transfer size
	if (((size == HWORD) && (address[0] != 0)) ||
	    ((size == WORD) && (address.range(1,0) != 0)))
    {
        cout<<"address is not aligned with the transfer size!"<<endl;
        cout<<"For 16-bit transfers, lowest bit of address must be 1'b0."<<endl;
        cout<<"For 32-bit transfers, lowest two bits of address must be 2'b00."<<endl;
        return false;
    }

    // if INCR4/8/16, it may cross 1KB boundary
	// WRAP8/16/32 transfers won't cross 1KB boundary
	if (burst == INCR4)
	{
	    if(size == BYTE)
	    {
	        if((address.range(9,2)) == 0xff && (address.range(1,0) != 0))
	        {
	            cout<<"time = "<<sc_core::sc_time_stamp()<<endl;
	            cout<<"Warning: 8-bit INCR4 transfer from address "<<address<<" will cross 1KB boundary!"<<endl;
	            return false;
	        }
	    }
	    else if(size == HWORD)
	    {
	        if((address.range(9,3) == 0x7f) && (address.range(2,0) != 0))
	        {
	            cout<<"time = "<<sc_core::sc_time_stamp()<<endl;
	            cout<<"Warning: 16-bit INCR4 transfer from address "<<address<<" will cross 1KB boundary!"<<endl;
	            return false;
	        }
	    }
	    else if(size == WORD)
	    {
	        if((address.range(9,4) == 0x3f) && (address.range(3,0) != 0))
	        {
	            cout<<"time = "<<sc_core::sc_time_stamp()<<endl;
	            cout<<"Warning: 32-bit INCR4 transfer from address "<<address<<" will cross 1KB boundary!"<<endl;
	            return false;
	        }
	    }
	}
	else if(burst == INCR8)
	{
	    if(size == BYTE)
	    {
	        if((address.range(9,3)) == 0x7f && (address.range(2,0) != 0))
	        {
	            cout<<"time = "<<sc_core::sc_time_stamp()<<endl;
	            cout<<"Warning: 8-bit INCR8 transfer from address "<<address<<" will cross 1KB boundary!"<<endl;
	            return false;
	        }
	    }
	    else if(size == HWORD)
	    {
	        if((address.range(9,4) == 0x3f) && (address.range(3,0) != 0))
	        {
	            cout<<"time = "<<sc_core::sc_time_stamp()<<endl;
	            cout<<"Warning: 16-bit INCR8 transfer from address "<<address<<" will cross 1KB boundary!"<<endl;
	            return false;
	        }
	    }
	    else if(size == WORD)
	    {
	        if((address.range(9,5) == 0x1f) && (address.range(4,0) != 0))
	        {
	            cout<<"time = "<<sc_core::sc_time_stamp()<<endl;
	            cout<<"Warning: 32-bit INCR8 transfer from address "<<address<<" will cross 1KB boundary!"<<endl;
	            return false;
	        }
	    }
	}
	else if(burst == INCR16)
	{
	    if(size == BYTE)
	    {
	        if((address.range(9,4)) == 0x3f && (address.range(3,0) != 0))
	        {
	            cout<<"time = "<<sc_core::sc_time_stamp()<<endl;
	            cout<<"Warning: 8-bit INCR16 transfer from address "<<address<<" will cross 1KB boundary!"<<endl;
	            return false;
	        }
	    }
	    else if(size == HWORD)
	    {
	        if((address.range(9,5) == 0x1f) && (address.range(4,0) != 0))
	        {
	            cout<<"time = "<<sc_core::sc_time_stamp()<<endl;
	            cout<<"Warning: 16-bit INCR16 transfer from address "<<address<<" will cross 1KB boundary!"<<endl;
	            return false;
	        }
	    }
	    else if(size == WORD)
	    {
	        if((address.range(9,6) == 0xf) && (address.range(5,0) != 0))
	        {
	            cout<<"time = "<<sc_core::sc_time_stamp()<<endl;
	            cout<<"Warning: 32-bit INCR16 transfer from address "<<address<<" will cross 1KB boundary!"<<endl;
	            return false;
	        }
	    }
	}
	return true;
}

// **********************************************************************
//   This function is aimed to get the next address according to
//   the control signals such as hburst_o and hsize_o
// **********************************************************************

sc_uint<BUSWIDTH> transmit_ca::update_addr(sc_uint<BUSWIDTH> address,
                  int burst,int size)
{
    sc_uint<BUSWIDTH> temp_address;
    sc_uint<BUSWIDTH> address_inc;

    // if SINGLE or INCR* just increment address according to size
	if ((burst == SINGLE) || (burst == INCR) || (burst == INCR4) ||
	    (burst == INCR8) || (burst == INCR16))
    {
        if(size == BYTE)
            temp_address=address+1;
        else if(size == HWORD)
            temp_address=address+2;
        else if(size == WORD)
            temp_address=address+4;
    }
    else if(burst == WRAP4)
    {
        if(size == BYTE)
        {
            address_inc=address.range(5,0)+1;
            temp_address=(address.range(31,2),address_inc.range(1,0));
        }
        else if(size == HWORD)
        {
            address_inc=address.range(5,0)+2;
            temp_address=(address.range(31,3),address_inc.range(2,0));
        }
        else if(size == WORD)
        {
            address_inc=address.range(5,0)+4;
            temp_address=(address.range(31,4),address_inc.range(3,0));
        }
    }
    else if(burst == WRAP8)
    {
        if(size == BYTE)
        {
            address_inc=address.range(5,0)+1;
            temp_address=(address.range(31,3),address_inc.range(2,0));
        }
        else if(size == HWORD)
        {
            address_inc=address.range(5,0)+2;
            temp_address=(address.range(31,4),address_inc.range(3,0));
        }
        else if(size == WORD)
        {
            address_inc=address.range(5,0)+4;
            temp_address=(address.range(31,5),address_inc.range(4,0));
        }
    }
    else if(burst == WRAP16)
    {
        if(size == BYTE)
        {
            address_inc=address.range(5,0)+1;
            temp_address=(address.range(31,4),address_inc.range(3,0));
        }
        else if(size == HWORD)
        {
            address_inc=address.range(5,0)+2;
            temp_address=(address.range(31,5),address_inc.range(4,0));
        }
        else if(size == WORD)
        {
            address_inc=address.range(5,0)+4;
            temp_address=(address.range(31,6),address_inc.range(5,0));
        }
    }
    return temp_address;
}
