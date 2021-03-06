/*
------------------------------------------------------------------------
--
-- File :                       ahb_lt.h
-- Author:                      Tao Li
-- Date :                       $Date: 2014/2/21 11:43:32 $
-- Version      :               $Revision: 1.0 $
-- Abstract     :               AHB interface top module(LT)
--                              This file describes the top module of AHB
--                              master interface in LT.
--
-- Modification History:
-- Date                 By      Version     Change Description
------------------------------------------------------------------------
*/

#ifndef AHB_LT_H_INCLUDED
#define AHB_LT_H_INCLUDED
#include "systemc.h"
#include "tlm.h"                                    // TLM headers
#include "amba_if.h"                                // amba interface
#include "traffic_transmit_lt.h"                    // AT initiator
#include "traffic_package.h"                        // traffic generator

class ahb_lt
  : public sc_core::sc_module
  , virtual public tlm::tlm_bw_transport_if<>  // backward non-blocking interface
{
public:

//********************************************************************
//  @fn ahb_lt::ahb_lt
//  @brief ahb_lt constructor
//  @details
//    ahb_lt module contains a traffic package module and an traffic
//    transmit module etc.
//********************************************************************
  ahb_lt
  ( sc_core::sc_module_name name              //< module name
  , const unsigned int  ID                    //< initiator ID
  , unsigned int        active_txn_count      //< Max number of active transactions
  );

private:

// Not Implemented for this example but required by the initiator socket
  void
  invalidate_direct_mem_ptr
  ( sc_dt::uint64      start_range
  , sc_dt::uint64      end_range
  );

  tlm::tlm_sync_enum
  nb_transport_bw
  ( tlm::tlm_generic_payload  &payload
  , tlm::tlm_phase            &phase
  , sc_core::sc_time          &delta
  );

// Member Variables/Objects

public:
  tlm::tlm_initiator_socket<> initiator_socket;	//< processor socket
  sc_core::sc_export<amba_if> ahb_export;

private:

  typedef tlm::tlm_generic_payload  *gp_ptr;   //< Generic Payload pointer
  sc_core::sc_fifo <gp_ptr>  m_request_fifo;   //< request SC FIFO
  sc_core::sc_fifo <gp_ptr>  m_response_fifo;  //< response SC FIFO

  const unsigned int         m_ID;             //< initiator ID

  traffic_transmit_lt      m_initiator;        //< TLM initiator instance
  traffic_package          m_traffic_gen;      //< traffic generator instance

};

#endif // AHB_LT_H_INCLUDED
