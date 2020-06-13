/*
------------------------------------------------------------------------
--
-- File :                       traffic_transmit_at.h
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

#ifndef TRAFFIC_TRANSMIT_AT_H_INCLUDED
#define TRAFFIC_TRANSMIT_AT_H_INCLUDED
#include "systemc.h"
#include "tlm.h"                                    // TLM headers
#include <map>                                      // STL map
#include "tlm_utils/peq_with_get.h"                 // Payload event queue FIFO
#include "amba_constants.h"

class traffic_transmit_at                           // TLM AT traffic_transmit
: public sc_core::sc_module                         // inherit from SC module base clase
, virtual public tlm::tlm_bw_transport_if<>         // inherit from TLM "backward interface"
{
    SC_HAS_PROCESS(traffic_transmit_at);

//*********************************************************
// Ports, exports and Sockets
//*********************************************************
    typedef tlm::tlm_generic_payload  *gp_ptr;        // generic payload
public:
    sc_core::sc_port<sc_core::sc_fifo_in_if  <gp_ptr> > request_in_port;
    sc_core::sc_port<sc_core::sc_fifo_out_if <gp_ptr> > response_out_port;
    tlm::tlm_initiator_socket<>                         initiator_socket;

//=============================================================================
//	@fn traffic_transmit_at
//	@brief Constructor for AT Initiator
//	@details
//		Generic AT Initiator
//		Constructor offers several parameters for customization
//=============================================================================

    traffic_transmit_at                             // constructor
    ( sc_core::sc_module_name name                  // module name
    , const unsigned int ID                         // initiator ID
    , sc_core::sc_time end_rsp_delay                // delay
    );

//=============================================================================
//  @fn initiator_thread
//  @brief Initiator thread
//  @details
//   This thread takes generic payloads (gp) from the request FIFO that connects
//   to the traffic generator and initiates. When the transaction completes the
//   gp is placed in the response FIFO to return it to the traffic generator.
//=============================================================================
private:
    void initiator_thread (void);                     // initiator thread

//=============================================================================
//  @fn at_target_4_phase::send_end_rsp_method
//  @brief Send end response method
//  @details
//   This routine takes transaction responses from the m_send_end_rsp_PEQ.
//   It contains the state machine to manage the communication path to the
//   targets.  This method is registered as an SC_METHOD with the SystemC
//   kernel and is sensitive to m_send_end_rsp_PEQ.get_event()
//=============================================================================
private:
    void send_end_rsp_method(void);                   // send end response method

//=============================================================================
//  @fn check_addr(sc_uint<BUSWIDTH> address,
//                  sc_uint<HBURST_WIDTH> burst,sc_uint<HSIZE_WIDTH> size);
//
//  @brief check_addr
//
//  @details
//   This function is aimed to find out if address is aligned with
//   the transfer size,and if INCR4/8/16, it may cross 1KB boundary
//	 WRAP4/8/16 transfers won't cross 1KB boundary
//=============================================================================
  bool check_addr(sc_uint<BUSWIDTH> address,
                  sc_uint<HBURST_WIDTH> burst,sc_uint<HSIZE_WIDTH> size);

//=============================================================================
//	@brief Implementation of call from targets.
//	@details
//		This is the ultimate destination of the nb_transport_bw call from
//		the targets after being routed trough a Bus
//=============================================================================
  tlm::tlm_sync_enum nb_transport_bw(               // nb_transport
    tlm::tlm_generic_payload& transaction,          // transaction
    tlm::tlm_phase&           phase,                // transaction phase
    sc_core::sc_time&         time);                // elapsed time

//==============================================================================
// Required but not implemented member methods
//==============================================================================
  void invalidate_direct_mem_ptr(                   // invalidate_direct_mem_ptr
    sc_dt::uint64 start_range,                      // start range
    sc_dt::uint64 end_range);                       // end range

//==============================================================================
// Private member variables and methods
//==============================================================================
private:

  enum previous_phase_enum
    {Rcved_UPDATED_enum    	                      // Received TLM_UPDATED d
    ,Rcved_ACCEPTED_enum    	                  // Received ACCEPTED
    ,Rcved_END_REQ_enum    	                      // Received TLM_BEGIN_RESP
    };

  typedef std::map<tlm::tlm_generic_payload *, previous_phase_enum>
                          waiting_bw_path_map;
  waiting_bw_path_map     m_waiting_bw_path_map;        // Wait backward path map
  sc_core::sc_event       m_enable_next_request_event;
  tlm_utils::peq_with_get<tlm::tlm_generic_payload>
                          m_send_end_rsp_PEQ;           // send end response PEQ
  unsigned int            m_ID;                         // initiator ID
  sc_core::sc_time        m_end_rsp_delay;              // end response delay
};

#endif // TRAFFIC_TRANSMIT_AT_H_INCLUDED
