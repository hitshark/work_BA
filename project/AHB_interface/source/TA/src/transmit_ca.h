/*
------------------------------------------------------------------------
--
-- File :                       transmit_ca.h
-- Author:                      Tao Li
-- Date :                       $Date: 2014/3/17 9:39:45 $
-- Version      :               $Revision: 1.0 $
-- Abstract     :               transaction level cycle current model
--                              of ahb interface
--                              This file describes the ahb interface in
--                              cycle accurate level using the TLM1.0 standard.
--
-- Modification History:
-- Date                 By      Version     Change Description
------------------------------------------------------------------------
*/

#ifndef TRANSMIT_CA_H_INCLUDED
#define TRANSMIT_CA_H_INCLUDED
#include "tlm.h"                                    // TLM headers
#include "systemc.h"
#include "ahb_slave_if.h"
#include "amba_constants.h"

class transmit_ca
: public sc_core::sc_module                         // inherit from SC module base clase
{
    SC_HAS_PROCESS(transmit_ca);

    // Ports, exports and Sockets
    typedef tlm::tlm_generic_payload  *gp_ptr;        // generic payload
public:
    sc_core::sc_port<sc_core::sc_fifo_in_if  <gp_ptr> > request_in_port;
    sc_core::sc_port<sc_core::sc_fifo_out_if <gp_ptr> > response_out_port;
    sc_core::sc_port<ahb_slave_if> trans_port;

    transmit_ca                                     // constructor
    ( sc_core::sc_module_name name                  // module name
    , const unsigned int ID                         // initiator ID
    );

// **********************************************************************
//   This function is aimed to find out if address is aligned with
//   the transfer size,and if INCR4/8/16, it may cross 1KB boundary
//	 WRAP4/8/16 transfers won't cross 1KB boundary.
// **********************************************************************

bool check_addr(sc_uint<BUSWIDTH> address,
                  int burst,int size);

// **********************************************************************
//   This function is aimed to get the next address according to
//   the control signals such as hburst_o and hsize_o
// **********************************************************************

sc_uint<BUSWIDTH> update_addr(sc_uint<BUSWIDTH> address,
                  int burst,int size);

private:
    void initiator_thread ();                     // initiator thread
    void ca_transmit_thread();                   // send end response method


// Private member variables and methods
//==============================================================================
private:
    unsigned int            m_ID;                         // initiator ID

// ******************************************************
// AHB Master Interface State Machine Encoding
// ******************************************************
    enum transmit_states{
                        AHBM_BREQ         =0x1,
                        AHBM_READ         =0x2,
                        AHBM_WRITE        =0x3
                        };
    transmit_states ahb_master_state;
// variables used to communication between threads
    bool rd_req;
    bool wr_req;
    bool trans_status;
    bool trans_flag;
    sc_core::sc_event trans_complete;
    int resp_state;

// registers
    int beat_count;
    int temp_beat;
    int temp_beat_for_retry;
    bool t_write;
    int t_trans;
    int t_burst;
    int t_size;
    unsigned int t_data;
    unsigned int t_addr;
    unsigned int temp_addr;

// read and write data buffer
    unsigned int t_rdata[BUFFER_SIZE];
    unsigned int t_wdata[BUFFER_SIZE];
};


#endif // TRANSMIT_CA_H_INCLUDED
