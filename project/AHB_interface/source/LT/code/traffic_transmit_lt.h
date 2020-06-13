/*
------------------------------------------------------------------------
--
-- File :                       traffic_transmit_lt.h
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

#ifndef TRAFFIC_TRANSMIT_LT_H_INCLUDED
#define TRAFFIC_TRANSMIT_LT_H_INCLUDED
#include "systemc.h"
#include "tlm.h"                                    // TLM headers
#include "amba_constants.h"

class traffic_transmit_lt                           // TLM LT traffic_transmit
: public sc_core::sc_module                         // inherit from SC module base clase
, virtual public tlm::tlm_bw_transport_if<>         // inherit from TLM "backward interface"
{
    SC_HAS_PROCESS(traffic_transmit_lt);

//*********************************************************
// Ports, exports and Sockets
//*********************************************************
    typedef tlm::tlm_generic_payload  *gp_ptr;        // generic payload
public:
    sc_core::sc_port<sc_core::sc_fifo_in_if  <gp_ptr> > request_in_port;
    sc_core::sc_port<sc_core::sc_fifo_out_if <gp_ptr> > response_out_port;
    tlm::tlm_initiator_socket<>                         initiator_socket;

//=============================================================================
//	@fn traffic_transmit_lt
//
//	@brief Constructor for LT Initiator
//
//	@details
//		Generic LT Initiator,
//		Constructor offers several parameters for customization
//
//=============================================================================

    traffic_transmit_lt                                  // constructor
    ( sc_core::sc_module_name name                       // module name
    , const unsigned int ID                              // initiator ID
    );

//=============================================================================
//  @fn initiator_thread
//
//  @brief Initiator thread
//
//  @details
//   This thread takes generic payloads (gp) from the request FIFO that connects
//   to the traffic generator and initiates. When the transaction completes the
//   gp is placed in the response FIFO to return it to the traffic generator.
//=============================================================================
private:
    void initiator_thread ();                     // initiator thread

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
//	@brief backward transmit interfaces
//
//	@details
//		Required but not implemented in LT module
//=============================================================================
  tlm::tlm_sync_enum nb_transport_bw(               // nb_transport
    tlm::tlm_generic_payload& transaction,          // transaction
    tlm::tlm_phase&           phase,                // transaction phase
    sc_core::sc_time&         time);                // elapsed time

  void invalidate_direct_mem_ptr(                   // invalidate_direct_mem_ptr
    sc_dt::uint64 start_range,                      // start range
    sc_dt::uint64 end_range);                       // end range

//*********************************************************
// Private member variables and methods
//*********************************************************
private:
  unsigned int            m_ID;                         // initiator ID
};


#endif // TRAFFIC_TRANSMIT_LT_H_INCLUDED
