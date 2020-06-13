/*
------------------------------------------------------------------------
--
-- File :                       traffic_transmit_lt.cpp
-- Author:                      Tao Li
-- Date :                       $Date: 2014/2/18 10:03:45 $
-- Version      :               $Revision: 1.0 $
-- Abstract     :               loosely timed traffic transmit model
--                              This file describes the LT transmit model
--                              which read transactions from request fifo
--                              and send it to target through initiator socket,
--                              when completed send the transaction
--                              back to response fifo
--
-- Modification History:
-- Date                 By      Version     Change Description
------------------------------------------------------------------------
*/

#include "traffic_transmit_lt.h"

using namespace sc_core;
using std::cout;
using std::endl;

//=============================================================================
//Constructor
//=============================================================================

traffic_transmit_lt::traffic_transmit_lt            // constructor
( sc_module_name name                               // module name
, const unsigned int ID                             // initiator ID
)
: sc_module           (name)                        // init module name
, initiator_socket    ("initiator_socket")          // init socket name
, m_ID                (ID)                          // init initiator ID
{
    // bind initiator to the export
    initiator_socket (*this);
    // register thread process
    SC_THREAD(initiator_thread);
}

//=============================================================================
//  Initiator thread
//=============================================================================
void traffic_transmit_lt::initiator_thread()   // initiator thread
{
	sc_core::wait(5,sc_core::SC_NS);
	sc_uint<HBURST_WIDTH> t_burst;
    sc_uint<HSIZE_WIDTH> t_size;
    sc_uint<BUSWIDTH> t_addr;
    sc_uint<BEAT_SIZE> beat_count;
	tlm::tlm_response_status gp_status;
    tlm::tlm_generic_payload *transaction_ptr;    // transaction pointer
    while (true)
    {
        // Read FIFO to get new transaction GP from the traffic generator
        while(request_in_port->num_available()>0)
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
            t_addr=sc_uint<BUSWIDTH>(addr_temp);    // store address
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

            sc_time delay         = SC_ZERO_TIME;       // Create delay objects

            // Make the blocking call and decode returned status
            initiator_socket->b_transport(*transaction_ptr, delay);
            gp_status = transaction_ptr->get_response_status();

            if(gp_status == tlm::TLM_OK_RESPONSE)
                wait(delay);
            response_out_port->write(transaction_ptr);    // send GP to output rsp fifo
        } //end while(request_in_port->num_available()>0)
		sc_core::wait(5,SC_NS);
    } // end while true
} // end initiator_thread

// **********************************************************************
//   This function is aimed to find out if address is aligned with
//   the transfer size,and if INCR4/8/16, it may cross 1KB boundary
//	 WRAP4/8/16 transfers won't cross 1KB boundary.
// **********************************************************************

bool traffic_transmit_lt::check_addr(sc_uint<BUSWIDTH> address,
                  sc_uint<HBURST_WIDTH> burst,sc_uint<HSIZE_WIDTH> size)
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

tlm::tlm_sync_enum traffic_transmit_lt::nb_transport_bw
    (tlm::tlm_generic_payload &trans
     ,tlm::tlm_phase &phase
     ,sc_core::sc_time &t)
{
    cout<<"nb_transport_bw: not implement!"<<endl;
    return tlm::TLM_COMPLETED;
}

void traffic_transmit_lt::invalidate_direct_mem_ptr  // invalidate_direct_mem_ptr
( sc_dt::uint64 start_range                          // start range
, sc_dt::uint64 end_range                            // end range
)
{
    cout<<"invalidate_direct_mem_ptr: not implemented!"<<endl;
}
