/*
------------------------------------------------------------------------
--
-- File :                       ca_transmit.cpp
-- Author:                      Tao Li
-- Date :                       $Date: 2014/3/1 17:50:45 $
-- Version      :               $Revision: 1.0 $
-- Abstract     :               cycle current model of ahb interface
--                              This file describes an adaptor which turns the
--                              AMBA 2.0 AHB interface in transaction level
--                              model(TLM) to cycle current model.
--
-- Modification History:
-- Date                 By      Version     Change Description
------------------------------------------------------------------------
*/

#include "ca_transmit.h"

// **********************************************************************
// This thread takes generic payloads (gp) from the request FIFO that connects
// to the traffic package. When the transaction completes the
// gp is placed in the response FIFO to return it to the traffic generator.
// **********************************************************************

void ca_transmit::initiator_thread()
{
    rd_req=false;
    wr_req=false;
    trans_status=true;
    tlm::tlm_generic_payload *transaction_ptr;    // transaction pointer
	sc_core::wait(5,sc_core::SC_NS);
    while (true)
    {
        // Read FIFO to Get new transaction GP from the traffic generator
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
}

// **********************************************************************
// This thread converts the informations extracted from the generic
// payloads (gp) to cycle current signals and receives the response signals
// from the AHB slave.
// **********************************************************************

void ca_transmit::transmit_thread(void)
{
    wait(hresetn.negedge_event());
	wait(3.03,SC_NS), hwdata_o=DEFAULT_DATA;
	wait(0.64,SC_NS), hwrite_o=READ;
	wait(0.42,SC_NS), htrans_o=IDLE;
	wait(0.15,SC_NS), hburst_o=SINGLE;
	wait(0.21,SC_NS), hbusreq_o=FALSE;
	wait(0.89,SC_NS), haddr_o=DEFAULT_ADDR;
	wait(0,SC_NS), hsize_o=WORD;
    busy_flag=0;
    ahb_master_state=AHBM_IDLE;
    wait(hresetn.posedge_event());

    while(true)
    {
        wait();
        switch(ahb_master_state)
        {
            case AHBM_IDLE:
            {
                // if read or write transaction occurs
                // then start the bus request process
                if(rd_req | wr_req)
                {
                    wait(4.45,SC_NS),hbusreq_o=TRUE;
                    ahb_master_state=AHBM_BREQ;         // turn to bus request state
                }
                break;
            }
            // start the first address phase
            // send out the address and control signals
            // if not INCR transmit type,then assert hbusreq_o FALSE
            // the arbitrator will use hburst_o signal to decide
            // how many beats is requested
            case AHBM_BREQ:
            {
                if(hgrant_i.read() & hready_i.read())
                {
					if(t_write==WRITE)                  // read or write
					{
						wait(3.67,SC_NS), hwrite_o=WRITE;
						ahb_master_state=AHBM_NSEQWR;
					}
					else
					{
						wait(3.67,SC_NS), hwrite_o=READ;
						ahb_master_state=AHBM_NSEQRD;
					}
					wait(0.42,SC_NS), htrans_o=NONSEQ;					// transmit type is set to nonsequential
					wait(0.15,SC_NS), hburst_o=t_burst;
					if(t_burst!=INCR)                   	// if burst type is INCR,then
														   	// it needs asserting hbusreq_o singal until the
														   	// last transmit starts
					{
						wait(0.21,SC_NS), hbusreq_o=FALSE;
					}
					wait(0.89,SC_NS), haddr_o=temp_addr;
					temp_addr=update_addr(temp_addr,t_burst,t_size);
					wait(0,SC_NS), hsize_o=t_size;
                }
                break;
            }

            // the end edge of address phase
            case AHBM_NSEQRD:
            {
                if(hready_i.read())
                // If it is single read cycle(hburst_o == SINGLE) OR
                // only one more data left from current read burst
                // go to WAIT(until data phase completes) as current data
                // is last data phase
                {
					if(!hgrant_i.read())                 // the grant is deprived from the master
                    {
						wait(4.09,SC_NS), htrans_o=IDLE;
						wait(1.25,SC_NS), haddr_o=DEFAULT_ADDR;
                        ahb_master_state=AHBM_LASTWR;
						break;
                    }
                    if(t_burst==SINGLE || beat_count==1)
                    {
                        wait(4.09,SC_NS), htrans_o=IDLE;
						wait(1.25,SC_NS), haddr_o=DEFAULT_ADDR;
                        ahb_master_state=AHBM_RDWAIT;
                    }
                    else
                    {
                        wait(4.09,SC_NS), htrans_o=SEQ;
						wait(1.25,SC_NS), haddr_o=temp_addr;
                        temp_addr=update_addr(temp_addr,t_burst,t_size);
                        ahb_master_state=AHBM_SEQRD;
                    }
                }
                break;
            }

            // consecutive transfers of burst read
            case AHBM_SEQRD:
            {
                if(hready_i.read() & (hresp_i.read()== OKAY))
                {
                    t_rdata[beat_count-1]=(unsigned int)hrdata_i.read(); //handle the read data
                    beat_count--;
					if(!hgrant_i.read())                 // the grant is deprived from the master
                    {
						wait(4.09,SC_NS), htrans_o=IDLE;
						wait(1.25,SC_NS), haddr_o=DEFAULT_ADDR;
                        ahb_master_state=AHBM_LASTRD;
						break;
                    }
                    if(htrans_o.read()==BUSY)
                    {
                        // resend address,the write data is not delivered
						wait(4.09,SC_NS), htrans_o=SEQ;
                        wait(1.25,SC_NS), haddr_o=haddr_o.read();
                        busy_flag=1;                    // when transmit type is busy,address
                                                        // is not refreshed at current cycle
                                                        // if hgrant is coincidently deprived at the same time
                                                        // then in LASTRD state
                                                        // it will not refresh the beat_count etc.
                    }
                    else if(htrans_o.read()==IDLE)      // IDLE state,the master is granted but won't
                                                        // want to do anything
                    {
                        wait(4.45,SC_NS), hbusreq_o=FALSE;
						wait(0.89,SC_NS), haddr_o=DEFAULT_ADDR;
                        ahb_master_state=AHBM_IDLE;
						trans_complete.notify();
						trans_status=FALSE;
						rd_req=FALSE;
                    }
                    else
                    {
                        if(beat_count>1)                // beat_count==2 indicates the last address phase
                        {
							wait(4.09,SC_NS), htrans_o=SEQ;
                            wait(1.25,SC_NS), haddr_o=temp_addr;
                            temp_addr=update_addr(temp_addr,t_burst,t_size);    // update the address
                        }
                        else if(beat_count==1)
                        {
                            ahb_master_state=AHBM_RDWAIT;
							wait(4.09,SC_NS),htrans_o=IDLE;
							wait(0.36,SC_NS),hbusreq_o=FALSE;                     // at the end edge of the address phase
							wait(0.89,SC_NS),haddr_o=DEFAULT_ADDR;
                        }
                    }
                }
                else if(!hready_i.read() && (hresp_i.read() == RETRY || hresp_i.read() == SPLIT))
                // when response is RETRY or SPLIT, set the address to orignal
                // start address and restart the request
                // set the haddr_o signal to DEFAULT_ADDR
                {
                    wait(4.09,SC_NS),htrans_o=IDLE;
                    wait(0.36,SC_NS),hbusreq_o=TRUE;
                    wait(0.89,SC_NS),haddr_o=DEFAULT_ADDR;
                    ahb_master_state=AHBM_BREQ;
                    temp_addr=t_addr;
					beat_count=temp_beat_for_retry;
                }
                else if(!hready_i.read() & (hresp_i.read()== EROR))
                // when response is ERROR,change to IDLE state
                {
                    ahb_master_state=AHBM_IDLE;
					wait(4.09,SC_NS),htrans_o=IDLE;
                    wait(0.36,SC_NS),hbusreq_o=FALSE;
					wait(0.89,SC_NS),haddr_o=DEFAULT_ADDR;
					trans_complete.notify();
					trans_status=FALSE;
					rd_req=FALSE;
                }
                break;
            }

         // last read data phase from current read transfer
         // (due to lost ownership of address bus)
         // once the current data phase is over(successful or not(retry/split))
         // master rearbitrates and starts with non-sequential again
            case AHBM_LASTRD:
            {
                if(hready_i.read() & (hresp_i.read() == OKAY))
                {
                    t_rdata[beat_count-1]=(unsigned int)hrdata_i.read(); // handle the read data
                    beat_count--;
                    if(beat_count==0)                      // beat_count==0 indicates that this
                                                           // is the last data phase,unnecessary to request again
					{
                        ahb_master_state=AHBM_IDLE;
						trans_complete.notify();
						trans_status=TRUE;
						rd_req=FALSE;
					}
                    else
                    {
                        wait(4.45,SC_NS),hbusreq_o=TRUE;
                        ahb_master_state=AHBM_BREQ;
                        t_burst=INCR;                   // refresh the burst type to INCR
                                                        // using INCR burst type to complete
                                                        // the rest transaction
                    }
                }
                else if(!hready_i.read() && (hresp_i.read() == RETRY || hresp_i.read() == SPLIT))
                {
                    wait(4.45,SC_NS),hbusreq_o=TRUE;
                    ahb_master_state=AHBM_BREQ;
					beat_count=temp_beat_for_retry;
                    temp_addr=t_addr;               // use the original address to restart
                                                    // if the RETRY or SPLIT response indicates that
                                                    // restart from current transmit
                                                    // then add t_burst=INCR and delete temp_addr=t_addr
                }
                else if(!hready_i.read() && (hresp_i.read() == EROR))
                {
                    wait(4.45,SC_NS),hbusreq_o=FALSE;
                    ahb_master_state=AHBM_IDLE;
					trans_complete.notify();
					trans_status=FALSE;
					rd_req=FALSE;
                }
                break;
            }

            // the end edge of data phase of either single read cycle
            // or last data phase of burst read cycle
            // use the hready_i and hresp_i signals to decide the next state
            case AHBM_RDWAIT:
            {
                if(hready_i.read() && hresp_i.read() == OKAY )
                {
                    t_rdata[beat_count-1]=(unsigned int)hrdata_i.read(); //handle the read data
                    beat_count--;
                    ahb_master_state=AHBM_IDLE;
					trans_complete.notify();
					trans_status=TRUE;
					rd_req=FALSE;
                }
                else if(!hready_i.read() && (hresp_i.read() == RETRY || (hresp_i.read() == SPLIT)))
                {
                    temp_addr=t_addr;
					beat_count=temp_beat_for_retry;
                    wait(4.45,SC_NS),hbusreq_o=TRUE;
                    ahb_master_state=AHBM_BREQ;
                }
                else if(!hready_i.read() && hresp_i.read() == EROR)
                {
                    wait(4.45,SC_NS),hbusreq_o=FALSE;
                    ahb_master_state=AHBM_IDLE;
					trans_complete.notify();
					trans_status=FALSE;
					rd_req=FALSE;
                }
                break;
            }

            // first address phase of write transfer
            case AHBM_NSEQWR:
            {
                if(hready_i.read())
                // If it is single write cycle(hburst_o == SINGLE) OR
                // only one more data left from current write burst
                // go to WAIT(until data phase completes) as current data
                // is last data phase
                {
					if(!hgrant_i.read())
					{
						wait(3.03,SC_NS),hwdata_o=t_wdata[beat_count-1];     // send write data from write data buffer
						wait(1.06,SC_NS),htrans_o=IDLE;
						wait(1.25,SC_NS),haddr_o=DEFAULT_ADDR;
                        ahb_master_state=AHBM_LASTWR;
						break;
					}
                    if(t_burst==SINGLE || beat_count==1) // If it is single write cycle go to WAIT as
                                                        // current data is last data phase
					{
						wait(3.03,SC_NS),hwdata_o=t_wdata[beat_count-1];     // send write data from write data buffer
						wait(1.06,SC_NS),htrans_o=IDLE;
						wait(1.25,SC_NS),haddr_o=DEFAULT_ADDR;
                        ahb_master_state=AHBM_WRWAIT;
					}
                    else
                    {
						wait(3.03,SC_NS),hwdata_o=t_wdata[beat_count-1];     // send write data from write data buffer
						wait(1.06,SC_NS),htrans_o=SEQ;
                        wait(1.25,SC_NS),haddr_o=temp_addr;
                        temp_addr=update_addr(temp_addr,t_burst,t_size);
                        ahb_master_state=AHBM_SEQWR;
                    }
                }
                break;
            }

            // sub-sequent address/data phases of burst write cycle
            case AHBM_SEQWR:
            {
                if(hready_i.read() && hresp_i.read()==OKAY)
                {
                    beat_count--;
                    if(htrans_o.read()==BUSY)
                    {
                        // resend address,the write data is not delivered
                        wait(3.03,SC_NS),hwdata_o=DEFAULT_DATA;
                        wait(1.06,SC_NS),htrans_o=SEQ;
						wait(1.25,SC_NS),haddr_o=haddr_o.read();
                        busy_flag=1;                    // when transmit type is busy,address and write data
                                                        // is not refreshed at current cycle
                                                        // if hgrant is coincidently deprived at the same time
                                                        // then in LASTWR state
                                                        // it will not refresh the beat_count etc.
                    }
                    else if(htrans_o.read()==IDLE)      // IDLE state,the master is granted but won't
                                                        // want to do anything
                    {
                        wait(4.45,SC_NS), hbusreq_o=FALSE;
						wait(0.89,SC_NS), haddr_o=DEFAULT_ADDR;
                        ahb_master_state=AHBM_IDLE;
						trans_complete.notify();
						trans_status=FALSE;
						wr_req=FALSE;
                    }
                    else
                    {
						if(!hgrant_i.read())                 // the grant is deprived from the master
						{
							wait(3.03,SC_NS),hwdata_o=t_wdata[beat_count-1];
							wait(1.06,SC_NS),htrans_o=IDLE;
							wait(1.25,SC_NS),haddr_o=DEFAULT_ADDR;
							ahb_master_state=AHBM_LASTWR;
							break;
						}
                        if(beat_count>1)                     // beat_count==2 indicates the last address phase
                        {
							wait(3.03,SC_NS),hwdata_o=t_wdata[beat_count-1];
							wait(1.06,SC_NS),htrans_o=SEQ;
                            wait(1.25,SC_NS),haddr_o=temp_addr;
                            temp_addr=update_addr(temp_addr,t_burst,t_size);    // update the address
                        }
                        else if(beat_count==1)
                        {
							wait(3.03,SC_NS), hwdata_o=t_wdata[beat_count-1];
							wait(1.06,SC_NS), htrans_o=IDLE;
							wait(0.36,SC_NS), hbusreq_o=FALSE;
							wait(0.89,SC_NS), haddr_o=DEFAULT_ADDR;
                            ahb_master_state=AHBM_WRWAIT;
                        }
                    }
                }
                else if(!hready_i.read() && (hresp_i.read() == RETRY || hresp_i.read() == SPLIT))
                // when response is RETRY or SPLIT, set the address to orignal
                // start address and restart the request
                // set the haddr_o signal to DEFAULT_ADDR
                {
                    wait(4.09,SC_NS),htrans_o=IDLE;
                    wait(0.36,SC_NS),hbusreq_o=TRUE;
                    wait(0.89,SC_NS),haddr_o=DEFAULT_ADDR;
                    ahb_master_state=AHBM_BREQ;
                    temp_addr=t_addr;
					beat_count=temp_beat_for_retry;
                }
                else if(!hready_i.read() & (hresp_i.read()== EROR))
                // when response is ERROR,change to IDLE state
                {
                    ahb_master_state=AHBM_IDLE;
					wait(4.09,SC_NS),htrans_o=IDLE;
                    wait(0.36,SC_NS),hbusreq_o=FALSE;
                    wait(0.89,SC_NS),haddr_o=DEFAULT_ADDR;
					trans_complete.notify();
					trans_status=FALSE;
					wr_req=FALSE;
                }
                break;
            }

            // last write data phase from current write transfer
            // (due to lost ownership of address bus)
            // once the current data phase is over(successful or not(retry/split))
            // master rearbitrates and starts with non-sequential again
            case AHBM_LASTWR:
            {
                if(hready_i.read() & (hresp_i.read() == OKAY))
                {
                    if(busy_flag)                           // transmit type is busy in last cycle
                                                            // address and write data is not refreshed
                    {
                        busy_flag=0;
                        wait(4.45,SC_NS),hbusreq_o=TRUE;
                        ahb_master_state=AHBM_BREQ;
                        t_burst=INCR;
                    }
                    else
                    {
                        beat_count--;
                        if(beat_count==0)                   // beat_count==0 indicates that this
                                                            // is the last data phase,unnecessary to request again
						{
                            ahb_master_state=AHBM_IDLE;
							trans_complete.notify();
							trans_status=TRUE;
							wr_req=FALSE;
						}
                        else
                        {
                            wait(4.45,SC_NS),hbusreq_o=TRUE;
                            ahb_master_state=AHBM_BREQ;
                            t_burst=INCR;                   // refresh the burst type to INCR
                                                            // using INCR burst type to complete
                                                            // the rest transaction
                        }
                    }
                }
                else if(!hready_i.read() && (hresp_i.read() == RETRY || hresp_i.read() == SPLIT))
                {
                    wait(4.45,SC_NS),hbusreq_o=TRUE;
                    ahb_master_state=AHBM_BREQ;
					beat_count=temp_beat_for_retry;
                    temp_addr=t_addr;               // use the original address to restart
                                                    // if the RETRY or SPLIT response indicates that
                                                    // restart from current transmit
                                                    // then add t_burst=INCR and delete temp_addr=t_addr
                }
                else if(!hready_i.read() && (hresp_i.read() == EROR))
                {
                    wait(4.45,SC_NS),hbusreq_o=FALSE;
                    ahb_master_state=AHBM_IDLE;
					trans_complete.notify();
					trans_status=FALSE;
					wr_req=FALSE;
                }
                break;
            }

            // the end edge of data phase of either single read cycle
            // or last data phase of burst write cycle
            // use the hready_i and hresp_i signals to decide the next state
            case AHBM_WRWAIT:
            {
                if(hready_i.read() && hresp_i.read() == OKAY )
                {
                    beat_count--;
                    ahb_master_state=AHBM_IDLE;
					trans_complete.notify();
					trans_status=TRUE;
					wr_req=FALSE;
                }
                else if(!hready_i.read() && (hresp_i.read() == RETRY || (hresp_i.read() == SPLIT)))
                {
                    temp_addr=t_addr;
					beat_count=temp_beat_for_retry;
                    wait(4.45,SC_NS),hbusreq_o=TRUE;
                    ahb_master_state=AHBM_BREQ;
                }
                else if(!hready_i.read() && hresp_i.read() == EROR)
                {
                    wait(4.45,SC_NS),hbusreq_o=FALSE;
                    ahb_master_state=AHBM_IDLE;
					trans_complete.notify();
					trans_status=FALSE;
					wr_req=FALSE;
                }
                break;
            }
            default:;
        }
    }
}

// **********************************************************************
//   This function is aimed to find out if address is aligned with
//   the transfer size,and if INCR4/8/16, it may cross 1KB boundary
//	 WRAP4/8/16 transfers won't cross 1KB boundary.
// **********************************************************************

bool ca_transmit::check_addr(sc_uint<BUSWIDTH> address,
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

// **********************************************************************
//   This function is aimed to get the next address according to
//   the control signals such as hburst_o and hsize_o
// **********************************************************************

sc_uint<BUSWIDTH> ca_transmit::update_addr(sc_uint<BUSWIDTH> address,
                  sc_uint<HBURST_WIDTH> burst,sc_uint<HSIZE_WIDTH> size)
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
