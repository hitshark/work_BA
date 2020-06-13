/*
------------------------------------------------------------------------
--
-- File :                       ahb_ca.h
-- Author:                      Tao Li
-- Date :                       $Date: 2014/3/24 18:39:11 $
-- Version      :               $Revision: 1.0 $
-- Abstract     :               timing accurate model of ahb interface
--                              This file describes the top module
--                              of AHB interface in timing
--                              accurate level.
--
-- Modification History:
-- Date                 By      Version     Change Description
------------------------------------------------------------------------
*/

#ifndef AHB_CA_H_INCLUDED
#define AHB_CA_H_INCLUDED
#include "systemc.h"
#include "tlm.h"                                    // TLM headers
#include "amba_if.h"                                // amba interface
#include "ca_transmit.h"                            // cycle accutate model
#include "traffic_package.h"                        // traffic generator

class ahb_ca
  : public sc_core::sc_module
{
public:

//=====================================================================
//  @fn ahb_ca::ahb_ca
//  @brief ahb_ca constructor
//  @details
//    ahb_ca module contains a traffic generator and an example
//    unique initiator module which is cycle accurate
//=====================================================================
  ahb_ca
  ( sc_core::sc_module_name name              //< module name
  , const unsigned int  ID                    //< initiator ID
  , unsigned int        active_txn_count      //< Max number of active transactions
  );

//Member Variables/Objects
public:
  ca_transmit                ca_initiator;      //< TLM initiator instance
  sc_core::sc_export<amba_if> ahb_export;

private:

  typedef tlm::tlm_generic_payload  *gp_ptr;   //< Generic Payload pointer

  sc_core::sc_fifo <gp_ptr>  m_request_fifo;   //< request SC FIFO
  sc_core::sc_fifo <gp_ptr>  m_response_fifo;  //< response SC FIFO

  const unsigned int         m_ID;             //< initiator ID

  traffic_package          m_traffic;      //< traffic package instance

};

#endif // AHB_CA_H_INCLUDED
