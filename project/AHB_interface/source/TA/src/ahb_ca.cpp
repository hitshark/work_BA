/*
------------------------------------------------------------------------
--
-- File :                       ahb_ca.cpp
-- Author:                      Tao Li
-- Date :                       $Date: 2014/3/17 13:32:11 $
-- Version      :               $Revision: 1.0 $
-- Abstract     :               transaction level cycle accurate model
--                              of ahb interface
--                              This file describes the top module
--                              of AHB interface in
--                              cycle accurate level using the TLM1.0 standard.
--
-- Modification History:
-- Date                 By      Version     Change Description
------------------------------------------------------------------------
*/

#include "ahb_ca.h"                             // Top traffic generator & initiator

// Constructor
ahb_ca::ahb_ca
( sc_core::sc_module_name name
, const unsigned int    ID
, unsigned int          active_txn_count
)
  :sc_module            (name) 	                // module name for top
  ,ahb_export           ("ahb_export")          // export
  ,m_ID                 (ID)                    // initiator ID
  ,ca_initiator         ("ca_initiator",ID)                       // Init initiator
  ,m_traffic                                    // Init traffic Generator
    ("m_traffic"
    ,ID
    ,active_txn_count                           // Max active transactions
    )
{
    // Bind ports to m_request_fifo between m_initiator and m_traffic_gen
    m_traffic.request_out_port       (m_request_fifo);
    ca_initiator.request_in_port     (m_request_fifo);

    // Bind ports to m_response_fifo between m_initiator and m_traffic_gen
    ca_initiator.response_out_port   (m_response_fifo);
    m_traffic.response_in_port       (m_response_fifo);

    ahb_export(m_traffic);
}


