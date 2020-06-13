/*
------------------------------------------------------------------------
--
-- File :                       ahb_lt.cpp
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

#include "ahb_lt.h"                         // Top traffic generator & initiator

using std::cout;
using std::endl;

// Constructor
ahb_lt::ahb_lt
( sc_core::sc_module_name name
, const unsigned int    ID
, unsigned int          active_txn_count
)
  :sc_module        (name) 	                // module name for top
  ,initiator_socket ("lt_initiator_socket") // TLM socket
  ,ahb_export        ("ahb_export")         // export
  ,m_ID             (ID)                    // initiator ID
  ,m_initiator                              // Init initiator
    ("m_initiator"
    ,ID
    )
  ,m_traffic_gen                            // Init traffic Generator
    ("m_traffic_gen"
    ,ID
    ,active_txn_count                       // Max active transactions
    )
{
    // Bind ports to m_request_fifo between m_initiator and m_traffic_gen
    m_traffic_gen.request_out_port  (m_request_fifo);
    m_initiator.request_in_port     (m_request_fifo);

    // Bind ports to m_response_fifo between m_initiator and m_traffic_gen
    m_initiator.response_out_port   (m_response_fifo);
    m_traffic_gen.response_in_port  (m_response_fifo);

    // Bind initiator-socket to initiator-socket hierarchical connection
    m_initiator.initiator_socket(initiator_socket);
    ahb_export(m_traffic_gen);
}

//*****************************************************************
//  @fn ahb_lt::invalidate_direct_mem_ptr
//  @brief Unused mandatory virtual implementation
//  @details
//    No DMI is implemented in this example so unused
//*****************************************************************
  void
  ahb_lt::invalidate_direct_mem_ptr
  ( sc_dt::uint64      start_range
  , sc_dt::uint64      end_range
  )
  {
      cout<<"not implement, for hierachical connection of initiator socket!"<<endl;
  } // end invalidate_direct_mem_ptr

//*****************************************************************
 //  @fn ahb_lt::nb_transport_bw
 //  @brief Unused mandatory virtual implementation
 //  @details
 //    Unused implementation from hierarchichal connectivity of
 //    Initiator sockets.
//*****************************************************************
tlm::tlm_sync_enum
ahb_lt::nb_transport_bw
( tlm::tlm_generic_payload  &payload
, tlm::tlm_phase            &phase
, sc_core::sc_time          &delta
)
{
    cout<<" Not implemented, for hierachical connection of initiator socket!"<<endl;
    return tlm::TLM_COMPLETED;
} // end nb_transport_bw
