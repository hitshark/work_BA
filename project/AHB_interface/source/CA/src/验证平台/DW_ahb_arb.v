/*
------------------------------------------------------------------------
--
--                    (C) COPYRIGHT 2004 SYNOPSYS, INC.
--                            ALL RIGHTS RESERVED
--
--  This software and the associated documentation are confidential and
--  proprietary to Synopsys, Inc.  Your use or disclosure of this
--  software is subject to the terms and conditions of a written
--  license agreement between you, or your company, and Synopsys, Inc.
--
--  The entire notice above must be reproduced on all authorized copies.
--  
-- File :                       DW_ahb_arb.v
-- Author:                      Ray Beechinor, Peter Gillen 
-- Date :                       $Date: 2004/06/09 14:24:41 $ 
-- Version      :               $Revision: 1.46 $ 
-- Abstract     :               AHB Arbiter.
--
--  Instantiates the following blocks:
--
--  * DW_arbiter_2t (Two-tier DW arbiter)
--  * DW_ahb_gctrl  (Grant Control Logic)
--  * DW_ahb_mask   (Mask Generation Logic)
--  * DW_ahb_ebt    (Early Burst Termination Control)
--  * DW_ahb_arbif  (AHB Arbiter Slave Interface) 
--               
--  DW_ahb_arb implements a two-tier arbitration scheme which complies
--  with the requirements of the AMBA specification.
--  The first tier of the arbitration is a user-programmable priority
--  scheme;  the second tier of arbitration is a "fair" scheme
--  which will make the arbitration decision when two or more masters
--  with the same user-programmed priority request bus access.
--           
-- Modification History:
-- Date                 By      Version Change  Description
-- =====================================================================
-- See CVS log
-- =====================================================================
*/

`include "DW_amba_constants.v"
`include "DW_ahb_constants.v"
`include "DW_ahb_cc_constants.v"

module DW_ahb_arb (
  hclk,
  hresetn,
  ahb_sc_arb,
  hready,
  hresp,
  hsel,
  haddr,
  hburst,
  hsize,
  htrans, 
  hwdata,
  hwrite,
  bus_hlock,
  bus_hbusreq, 
  bus_hsplit,
  hmaster_data,
  pause,
  ahb_big_endian,
  
  bus_hgrant,
  ahbarbint,
  hready_resp_s0,
  hresp_s0,
  hrdata_s0,
  hmaster,
  hmastlock,
  wt_count_m15,
  wt_count_m14,
  wt_count_m13,
  wt_count_m12,
  wt_count_m11,
  wt_count_m10,
  wt_count_m9,
  wt_count_m8,
  wt_count_m7,
  wt_count_m6,
  wt_count_m5,
  wt_count_m4,
  wt_count_m3,
  wt_count_m2,
  wt_count_m1,
  ahb_wt_mask,
  ahb_wt_aps
);

  // physical parameters
  parameter haddr_width = `HADDR_WIDTH;       // 32, 64
  parameter ahb_data_width = `AHB_DATA_WIDTH; // 32, 64, 128, 256
  parameter big_endian = `BIG_ENDIAN;         // 0, 1

  input                        hclk;
  input                        hresetn;
  input                        ahb_sc_arb;
  input                        hready;
  input [`HRESP_WIDTH-1:0]     hresp;
  input                        hsel;
  input [haddr_width-1:0]     haddr;
  input [`HBURST_WIDTH-1:0]    hburst;
  input [`HSIZE_WIDTH-1:0]     hsize;
  input [`HTRANS_WIDTH-1:0]    htrans;
  input [ahb_data_width-1:0]  hwdata;
  input                        hwrite;
  input [`NUM_AHB_MASTERS:0]   bus_hlock;
// All bus requests are concatenated into a single bus.
  input [`NUM_AHB_MASTERS:0]   bus_hbusreq;
  input [`SPLITBUS_WIDTH-1:0]  bus_hsplit;
// Which master has control of the data bus
  input [`HMASTER_WIDTH-1:0]   hmaster_data;
// When set then pause mode is enabled.
  input                        pause;
  input                        ahb_big_endian;
  
  output [`NUM_AHB_MASTERS:0]  bus_hgrant;
  output                       ahbarbint;
// Arbiter slave response to master
  output                       hready_resp_s0;
// Arbiter slave transfer response to master
  output [`HRESP_WIDTH-1:0]    hresp_s0;
  output [ahb_data_width-1:0] hrdata_s0;
  output [`HMASTER_WIDTH-1:0]  hmaster;
  output                       hmastlock;
  output [`AHB_CCL_WIDTH-1:0]  wt_count_m15;
  output [`AHB_CCL_WIDTH-1:0]  wt_count_m14;
  output [`AHB_CCL_WIDTH-1:0]  wt_count_m13;
  output [`AHB_CCL_WIDTH-1:0]  wt_count_m12;
  output [`AHB_CCL_WIDTH-1:0]  wt_count_m11;
  output [`AHB_CCL_WIDTH-1:0]  wt_count_m10;
  output [`AHB_CCL_WIDTH-1:0]  wt_count_m9;
  output [`AHB_CCL_WIDTH-1:0]  wt_count_m8;
  output [`AHB_CCL_WIDTH-1:0]  wt_count_m7;
  output [`AHB_CCL_WIDTH-1:0]  wt_count_m6;
  output [`AHB_CCL_WIDTH-1:0]  wt_count_m5;
  output [`AHB_CCL_WIDTH-1:0]  wt_count_m4;
  output [`AHB_CCL_WIDTH-1:0]  wt_count_m3;
  output [`AHB_CCL_WIDTH-1:0]  wt_count_m2;
  output [`AHB_CCL_WIDTH-1:0]  wt_count_m1;
  output [`NUM_AHB_MASTERS:1]  ahb_wt_mask;
  output                       ahb_wt_aps;

  wire                         iahb_wt_aps;

  wire [(4*(`NUM_INT_MASTERS))-1:0] bus_priority;
  wire [`HSPLIT_WIDTH-1:0]     hsplit;
  wire [9:0]                   ebtcount;
  wire [`HMASTER_WIDTH-1:0]    def_mst;
  wire [`NUM_AHB_MASTERS:0]    maskmaster;
  wire [`NUM_AHB_MASTERS:0]    mask;
  wire [`NUM_AHB_MASTERS:0]    tokenmask;
  wire [`NUM_AHB_MASTERS:0]    grant;
  wire [`NUM_AHB_MASTERS:0]    grant_2t;
  wire [`NUM_AHB_MASTERS:0]    grant_3t;
  wire [`NUM_AHB_MASTERS:0]    int_grant_3t;
  wire                         parked;
  wire                         parked_2t;
  wire                         parked_3t;
  wire                         int_parked_3t;
  wire                         ebten;
  wire                         clr_arbint;
  wire                         set_ebt;
  wire                         new_tfr;
  wire                         locked;
  reg [255:0]                  int_hwdata;
  wire                         int_ahbarbint;
  wire                         int_set_ebt;
  wire [`NUM_AHB_MASTERS:0]    ibus_hbusreq;

  wire [`BUS_AHB_CCL_WIDTH-1:0] bus_ahb_icl;
  wire                          wten;
  wire [`AHB_TCL_WIDTH-1:0]     ahb_itcl;
  wire                          est;
  wire [`HMASTER_WIDTH-1:0]     ihmaster;
  wire                          csilt;
  wire                          ltip;
  wire                          grant_changed;
  
// When the mode is not configured do not want any logic used so need
// to free up the regsiter.

  assign ahb_wt_mask[`NUM_AHB_MASTERS:1] = 
           (`AHB_WTEN == 1) ? tokenmask[`NUM_AHB_MASTERS:1] : {`NUM_AHB_MASTERS{1'b0}};
  assign ahb_wt_aps = (`AHB_WTEN == 1) ? iahb_wt_aps : 1'b0;

  DW_ahb_arbif
   #(haddr_width, ahb_data_width, big_endian)
   U_arbif (
    .hclk             (hclk),
    .hresetn          (hresetn),
    .hsel             (hsel),
    .hready           (hready),
    .hmaster          (ihmaster),
    .set_ebt          (set_ebt),
    .haddr            (haddr),
    .htrans           (htrans),
    .hsize            (hsize),
    .hwdata           (int_hwdata),
    .hwrite           (hwrite),
    .ahb_big_endian   (ahb_big_endian),
    
    .hready_resp_s0   (hready_resp_s0),
    .hresp_s0         (hresp_s0),
    .hrdata_s0        (hrdata_s0),
    .clr_arbint       (clr_arbint),
    .ebtcount         (ebtcount),
    .ebten            (ebten),
    .def_mst          (def_mst),
    .bus_priority     (bus_priority),
    .maskmaster       (maskmaster),
    .wten             (wten),
    .bus_ahb_icl      (bus_ahb_icl),
    .ahb_itcl         (ahb_itcl)
  );

  always @(hwdata)
  begin : int_wdata_PROC
    int_hwdata = 256'b0;
    int_hwdata[ahb_data_width-1:0] = hwdata;
  end
//
// Early Burst Termination
//
  DW_ahb_ebt
  
   U_ebt (
    .hclk             (hclk),
    .hresetn          (hresetn),
    .ebtcount         (ebtcount),
    .ebten            (ebten),
    .clr_arbint       (clr_arbint),
    .hready           (hready),
    .new_tfr          (new_tfr),
    .grant_changed    (grant_changed),
    .ltip             (ltip),

    .ahbarbint        (int_ahbarbint),
    .set_ebt          (int_set_ebt)
  );

  assign set_ebt   = (`EBTEN == 0 ) ? 1'b0 : int_set_ebt;
  assign ahbarbint = (`EBTEN == 0 ) ? 1'b0 : int_ahbarbint;

//
// Mask 
//
  DW_ahb_mask
  
   U_mask (
    .hclk             (hclk),
    .hresetn          (hresetn),
    .hresp            (hresp),
    .bus_hsplit       (bus_hsplit),
    .hmaster_data     (hmaster_data),
    .maskmaster       (maskmaster),
    .bus_priority     (bus_priority),
    .ltip             (ltip),
    .csilt            (csilt),
    .hready           (hready),

    .est              (est),
    .hsplit           (hsplit),
    .mask             (mask)

  );

//
// Grant Control
//
  DW_ahb_gctrl
  
   U_gctrl (
    .hclk             (hclk),
    .hresetn          (hresetn),
    .ahb_sc_arb       (ahb_sc_arb),
    .bus_hlock        (bus_hlock),
    .def_mst          (def_mst),
    .pause            (pause),
    .hburst           (hburst),
    .hready           (hready),
    .hresp            (hresp),
    .hsplit           (hsplit),
    .htrans           (htrans),
    .grant_2t         (grant_2t),
    .parked_2t        (parked_2t),
    .mask             (mask),
    .set_ebt          (set_ebt),
    .est              (est),
    .grant_changed    (grant_changed),
    .ltip             (ltip),
    .csilt            (csilt),
    .bus_hbusreq      (bus_hbusreq),
    .ibus_hbusreq     (ibus_hbusreq),
    .bus_hgrant       (bus_hgrant),
    .hmastlock        (hmastlock),
    .hmaster          (ihmaster),
    .new_tfr          (new_tfr)

  );
//
// Two-tier DW arbiter.  This monitors all requests and indicates the 
// winner of each arbitration decision
//
// Arbiter parameters are as follows:
// `NUM_INT_MASTERS : number of masters in the system plus dummy master
// 4 : width of priority input vector
// 1 : park the arbiter when no master is requesting
// 0 : index of master granted the bus when no master is requesting
// 0 : outputs of DW_arbiter_2t are unregistered
//
  DW_arbiter_2t #(`NUM_INT_MASTERS,4,1,0,0) U_arb_2t (
    .clk              (hclk),
    .rst_n            (hresetn),
    .request          (ibus_hbusreq),
    .priority         (bus_priority),
    .lock             ({(`NUM_AHB_MASTERS+1){1'b0}}),
    .mask             (mask),
    .parked           (parked),
    .granted          (),
    .locked           (),
    .grant            (grant),
    .grant_index      ()
  );


  assign int_grant_3t  = (`AHB_WTEN == 1'b1) ? grant_3t  : {`NUM_AHB_MASTERS{1'b0}};
  assign int_parked_3t = (`AHB_WTEN == 1'b1) ? parked_3t : 1'b0;

  DW_ahb_gating
   U_gating (
    .wten      (wten),
    .grant_2t  (grant_2t),
    .parked_2t (parked_2t),
    .grant     (grant),
    .parked    (parked),
    .grant_3t  (int_grant_3t),
    .parked_3t (int_parked_3t)
  );

  assign hmaster = ihmaster;

endmodule
