/*
------------------------------------------------------------------------
--
-- File :                       ca_transmit.h
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

#ifndef CA_TRANSMIT_H_INCLUDED
#define CA_TRANSMIT_H_INCLUDED

#include "systemc.h"
#include "tlm.h"
#include "amba_constants.h"

SC_MODULE(ca_transmit){

// ******************************************************
// AHB Master Interface
// ******************************************************
  sc_in<bool> clk;                          // This clock times all bus transfers
  sc_in<bool> hresetn;                      // AHB reset signal

  sc_in<bool> hready_i;                     // when active indicates that
                                            // a transfer has finished on the bus
  sc_in<sc_uint<HRESP_WIDTH> > hresp_i;     // transfer response from AHB Slave
                                            // (OKAY,ERROR,RETRY,SPLIT)
  sc_in<bool> hgrant_i;                     // bus grant from AHB Arbiter

  sc_out<bool> hbusreq_o;                   // bus request to AHB Arbiter
  sc_out<sc_uint<HTRANS_WIDTH> > htrans_o;  // type of current transfer
                                            // (NONSEQ,SEQ,IDLE,BUSY)
  sc_out<bool> hwrite_o;                    // type of current transfer,Rd/Wr

  sc_out<sc_uint<HSIZE_WIDTH> > hsize_o;    // size of current transfer
  sc_out<sc_uint<HBURST_WIDTH> > hburst_o;  // Burst type
  sc_out<sc_uint<BUSWIDTH> > haddr_o;       // Address out onto AHB for Rd/Wr

  sc_out<sc_uint<BUSWIDTH> > hwdata_o;      // Write data out to AHB for Rx
  sc_in<sc_uint<BUSWIDTH> > hrdata_i;       // Read data from AHB for Tx


// ******************************************************
// AHB Master Interface State Machine Encoding
// ******************************************************
  enum transmit_states{
                        AHBM_IDLE         =0x0,
                        AHBM_BREQ         =0x1,
                        AHBM_NSEQRD       =0x2,
                        AHBM_SEQRD        =0x3,
                        AHBM_RDWAIT       =0x5,
                        AHBM_LASTRD       =0x6,
                        AHBM_NSEQWR       =0x7,
                        AHBM_SEQWR        =0x8,
                        AHBM_WRWAIT       =0x9,
                        AHBM_LASTWR       =0xA
                      };

// ******************************************************
// Internal signals and variables declarations
// ******************************************************
  // ahb_master_state is used to model the state machine
  sc_signal<transmit_states> ahb_master_state;

  // variables used to communication between threads
  bool rd_req;
  bool wr_req;
  bool trans_status;
  unsigned int busy_flag;
  sc_core::sc_event trans_complete;

  // registers
  sc_uint<5> beat_count;
  sc_uint<5> temp_beat;
  sc_uint<5> temp_beat_for_retry;
  bool t_write;
  sc_uint<HTRANS_WIDTH> t_trans;
  sc_uint<HBURST_WIDTH> t_burst;
  sc_uint<HSIZE_WIDTH> t_size;
  sc_uint<BUSWIDTH> t_addr;
  sc_uint<BUSWIDTH> temp_addr;

  // read and write data buffer
  unsigned int t_rdata[BUFFER_SIZE];
  unsigned int t_wdata[BUFFER_SIZE];

  // request fifo and response fifo
  typedef tlm::tlm_generic_payload  *gp_ptr;        // generic payload
  sc_core::sc_port<sc_core::sc_fifo_in_if  <gp_ptr> > request_in_port;
  sc_core::sc_port<sc_core::sc_fifo_out_if <gp_ptr> > response_out_port;

// ******************************************************
// initiator thread and transmit thread
// ******************************************************

//=============================================================================
//  @fn ca_transmit::initiator_thread
//
//  @brief Initiator thread
//
//  @details
//   This thread takes generic payloads (gp) from the request FIFO that connects
//   to the traffic package. When the transaction completes the
//   gp is placed in the response FIFO to return it to the traffic generator.
//=============================================================================
  void initiator_thread (void);

//=============================================================================
//  @fn ca_transmit::transmit_thread
//
//  @brief transmit_thread
//
//  @details
//   This thread converts the informations extracted from the generic
//   payloads (gp) to cycle current signals and receives the response signals
//   from the AHB slave.
//=============================================================================
  void transmit_thread(void);

//=============================================================================
//  @fn ca_transmit::check_addr(sc_uint<BUSWIDTH> address,
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
//  @fn ca_transmit::update_addr(sc_uint<BUSWIDTH> address,
//                  sc_uint<HBURST_WIDTH> burst,sc_uint<HSIZE_WIDTH> size);
//
//  @brief update_addr
//
//  @details
//   This function is aimed to get the next address according to
//   the control signals such as hburst_o and hsize_o
//=============================================================================
  sc_uint<BUSWIDTH> update_addr(sc_uint<BUSWIDTH> address,
                  sc_uint<HBURST_WIDTH> burst,sc_uint<HSIZE_WIDTH> size);

// ******************************************************
// constructor
// ******************************************************

  SC_CTOR(ca_transmit)
  {
      SC_THREAD(initiator_thread);
      SC_THREAD(transmit_thread);
      sensitive_pos<<clk;
  }
};

#endif