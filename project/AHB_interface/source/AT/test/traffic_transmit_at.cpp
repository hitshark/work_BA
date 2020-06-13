/*
------------------------------------------------------------------------
--
-- File :                       traffic_transmit_at.cpp
-- Author:                      Tao Li
-- Date :                       $Date: 2014/2/23 10:12:45 $
-- Version      :               $Revision: 1.0 $
-- Abstract     :               approximately timed traffic transmit model
--                              This file describes the AT transmit model
--                              which read transactions from request fifo
--                              and send it to target through initiator socket,
--                              when completed send the transaction
--                              back to response fifo
--
-- Modification History:
-- Date                 By      Version     Change Description
------------------------------------------------------------------------
*/

#include "traffic_transmit_at.h"                         // Our header

using namespace sc_core;
using std::cout;
using std::endl;

//=============================================================================
//Constructor
//=============================================================================

traffic_transmit_at::traffic_transmit_at            // constructor
( sc_module_name name                               // module name
, const unsigned int ID                             // initiator ID
, sc_core::sc_time end_rsp_delay                    // delay
)
: sc_module           (name)                        // init module name
, initiator_socket    ("initiator_socket")          // init socket name
, m_send_end_rsp_PEQ  ("send_end_rsp_PEQ")          // init PEQ name
, m_ID                (ID)                          // init initiator ID
, m_end_rsp_delay     (end_rsp_delay)               // init end response delay
{
    // bind initiator to the export
    initiator_socket (*this);

    // register thread process
    SC_THREAD(initiator_thread);

    // register method process
    SC_METHOD(send_end_rsp_method);
    sensitive << m_send_end_rsp_PEQ.get_event();
    dont_initialize();
}

//=============================================================================
//  Initiator thread
//=============================================================================

void traffic_transmit_at::initiator_thread(void)   // initiator thread
{
	sc_core::wait(5,sc_core::SC_NS);
	sc_uint<HBURST_WIDTH> t_burst;
    sc_uint<HSIZE_WIDTH> t_size;
    sc_uint<BUSWIDTH> t_addr;
    sc_uint<BEAT_SIZE> beat_count;
    tlm::tlm_generic_payload *transaction_ptr;    // transaction pointer
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

            tlm::tlm_phase phase  = tlm::BEGIN_REQ;     // Create phase objects
            sc_time delay         = SC_ZERO_TIME;       // Create delay objects

//-----------------------------------------------------------------------------
// Make the non-blocking call and decode returned status (tlm_sync_enum)
//-----------------------------------------------------------------------------
            tlm::tlm_sync_enum
            return_value = initiator_socket->nb_transport_fw(*transaction_ptr, phase, delay);
            switch (return_value)
            {

//-----------------------------------------------------------------------------
//  The target returned COMPLETED this is a single phase transaction
//    Wait the annotated delay
//    Return the transaction to the traffic generator
//-----------------------------------------------------------------------------
                case tlm::TLM_COMPLETED:
                {
                    wait(delay + m_end_rsp_delay);              // wait the annotated delay
                    response_out_port->write(transaction_ptr);  // return txn to traffic gen
                    break;
                }// end case TLM_COMPLETED

//-----------------------------------------------------------------------------
// Target returned UPDATED this will be 2 phase transaction
//    Add the transaction pointer to the waiting backward path map
//    Set tracking enum to Rcved_UPDATED
//    Wait the annotated delay
//-----------------------------------------------------------------------------
                case tlm::TLM_UPDATED:
                {
                    m_waiting_bw_path_map.insert(std::make_pair(transaction_ptr,Rcved_UPDATED_enum));
                    wait(m_enable_next_request_event);                     // wait the annotated delay
                    break;
				} // end case TLM_UPDATED

//-----------------------------------------------------------------------------
//  Target returned ACCEPTED this will be 4 phase transaction
//    Add the transaction pointer to the waiting backward path map
//    Set tracking enum to Rcved_END_REQ
//    When the END REQUEST RULE is active wait for the target to response
//-----------------------------------------------------------------------------
                case tlm::TLM_ACCEPTED:
                {
                    //  use map to track transaction including current state information
                    m_waiting_bw_path_map.insert (std::make_pair (transaction_ptr,Rcved_ACCEPTED_enum));
                    wait (m_enable_next_request_event);
                    break;
                } // end case TLM_ACCEPTED

//-----------------------------------------------------------------------------
// All case covered default
//-----------------------------------------------------------------------------
                default:
                {
                    cout<<"unexpected response to BEGIN_REQ!"<<endl;
                    break;
                }
            } // end case
        } //end while(request_in_port->num_available()>0)
		sc_core::wait(5,SC_NS);
    } // end while true
} // end initiator_thread

//=============================================================================
//  @fn nb_transport_bw
//  @brief non-blocking transport from targets
//=============================================================================
tlm::tlm_sync_enum
traffic_transmit_at::nb_transport_bw                    // inbound nb_transport_bw
( tlm::tlm_generic_payload&  transaction_ref            // generic payload
 , tlm::tlm_phase&            phase                     // tlm phase
 , sc_time&                   delay                     // delay
)
{
    tlm::tlm_sync_enum        status = tlm::TLM_COMPLETED;  // return status reject by default
    // Check waiting backward path map of valid transaction
    waiting_bw_path_map::iterator transaction_pair ;        // create interator for map
    transaction_pair  = m_waiting_bw_path_map.find(&transaction_ref);

    if (transaction_pair == m_waiting_bw_path_map.end())
    {

//-----------------------------------------------------------------------------
//  The transaction pointer used by the backward path call does not belong
//  to this initiator, this is a major error
//-----------------------------------------------------------------------------
        cout<<"received invalid transaction pointer!"<<endl;
    }
//-----------------------------------------------------------------------------
//  Normal operation
//    Decode backward path phase
//-----------------------------------------------------------------------------
    else
    {
         switch (phase)
         {
//-----------------------------------------------------------------------------
// Target has responded with END_REQ this is a 4 phase transaction
//    Verify the previous current tracking enum is Rcved_ACCEPTED_enum
//    Set tracking enum to Rcved_END_REQ
//-----------------------------------------------------------------------------
            case tlm::END_REQ:
            {
                if (transaction_pair->second == Rcved_ACCEPTED_enum)
                {
                    m_enable_next_request_event.notify(SC_ZERO_TIME);
                    transaction_pair->second = Rcved_END_REQ_enum;
                    status = tlm::TLM_ACCEPTED;
                }
                else
                {
                    cout<<"unexpected END_REQ!"<<endl;
                }
                break;
            }

//-----------------------------------------------------------------------------
//  Target has responded with BEGIN_RESP
//    The style could be 2,3 or 4 phase
//	  Decode the previous tracking enum
//-----------------------------------------------------------------------------
            case tlm::BEGIN_RESP:
            {
//-----------------------------------------------------------------------------
// Respond to begin-response - 2 phase style
//-----------------------------------------------------------------------------
                if( transaction_pair->second == Rcved_UPDATED_enum)
                {
                    m_enable_next_request_event.notify(SC_ZERO_TIME); // release reqeuster thread
                    m_waiting_bw_path_map.erase(&transaction_ref);    // erase from map

                    phase   = tlm::END_RESP;                          // set appropriate return phase
                    delay   = m_end_rsp_delay;                        // wait for the response delay
                    status  = tlm::TLM_COMPLETED;                     // return status

                    response_out_port->write(&transaction_ref);     // transaction to rsp fifo port
                }

//-----------------------------------------------------------------------------
//  Respond to begin-response when the target has omitted the
//  end-request timing-point - 3 phase style
//-----------------------------------------------------------------------------
                else if ( transaction_pair->second == Rcved_ACCEPTED_enum)
                {
                    m_waiting_bw_path_map.erase(&transaction_ref);    // erase from map
                    // use payload event queue to schedule sending end response
                    m_send_end_rsp_PEQ.notify (transaction_ref, m_end_rsp_delay);
                    m_enable_next_request_event.notify(SC_ZERO_TIME);
                    status = tlm::TLM_ACCEPTED;
                }

//-----------------------------------------------------------------------------
// Respond to begin-response - 4 phase style
//-----------------------------------------------------------------------------
                else if ( transaction_pair->second == Rcved_END_REQ_enum)
                {
                    m_waiting_bw_path_map.erase(&transaction_ref);    // erase from map
                    // use payload event queue to schedule sending end response
                    m_send_end_rsp_PEQ.notify (transaction_ref, m_end_rsp_delay);
                    status = tlm::TLM_ACCEPTED;
                }

//-----------------------------------------------------------------------------
                else
                {
                    cout<<"unexpected begin_resp!"<<endl;
                }
                break;
            }  // end case BEGIN_RESP
//-----------------------------------------------------------------------------
// Invalid phase for backward path
//-----------------------------------------------------------------------------
            case tlm::BEGIN_REQ:
            case tlm::END_RESP:
            {
                cout<<"illegal phase on backward path!"<<endl;
                break;
            }
//-----------------------------------------------------------------------------
// Unknown phase on backward path
//-----------------------------------------------------------------------------
            default:
            {
                cout<<"unknown phase on the backward path!"<<endl;
                break;
            }
        } // end switch (phase)
    }// end else
    return status;
} // end backward nb transport

//=============================================================================
//  @fn traffic_transmit::send_end_rsp_method
//  @brief send end response method
//  @details This method is scheduled to send the end-response timing point.
//           It is sensitive to the m_send_end_rsp_PEQ.get_event() event.
//=============================================================================
void traffic_transmit_at::send_end_rsp_method(void)    // send end response method
{
    tlm::tlm_generic_payload* transaction_ptr;

//-----------------------------------------------------------------------------
//  Process all transactions scheduled for current time a return value of NULL
//  indicates that the PEQ is empty at this time
//-----------------------------------------------------------------------------
    while ((transaction_ptr = m_send_end_rsp_PEQ.get_next_transaction()) != NULL)
    {
        tlm::tlm_phase phase  = tlm::END_RESP;          // set the appropriate phase
        sc_time delay         = SC_ZERO_TIME;
        // call begin response and then decode return status
        tlm::tlm_sync_enum
        return_value = initiator_socket->nb_transport_fw(*transaction_ptr, phase, delay);

        switch (return_value)
        {
            case tlm::TLM_COMPLETED:                          // transaction complete
            {
                response_out_port->write(transaction_ptr);    // send GP to output rsp fifo
                break;
            }
            case tlm::TLM_ACCEPTED:
            case tlm::TLM_UPDATED:
            default:
            {
                cout<<"unknow return value for end_resp!"<<endl;
                break;
            }
        } // end switch
    } // end while
    return;
} // end send_end_rsp_method

// **********************************************************************
//   This function is aimed to find out if address is aligned with
//   the transfer size,and if INCR4/8/16, it may cross 1KB boundary
//	 WRAP4/8/16 transfers won't cross 1KB boundary.
// **********************************************************************

bool traffic_transmit_at::check_addr(sc_uint<BUSWIDTH> address,
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

//=============================================================================
//  @fn traffic_transmit::invalidate_direct_mem_ptr
//  @brief invalidate direct memory pointer Not implemented
//=============================================================================

void traffic_transmit_at::invalidate_direct_mem_ptr  // invalidate_direct_mem_ptr
( sc_dt::uint64 start_range                       // start range
, sc_dt::uint64 end_range                         // end range
)
{
    cout<<"invalidate_direct_mem_ptr: not implemented!"<<endl;
}
