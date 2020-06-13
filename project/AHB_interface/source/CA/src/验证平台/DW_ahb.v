/*
------------------------------------------------------------------------
--
--                  (C) COPYRIGHT 2001-2004 SYNOPSYS, INC.
--                            ALL RIGHTS RESERVED
--
--  This software and the associated documentation are confidential and
--  proprietary to Synopsys, Inc.  Your use or disclosure of this
--  software is subject to the terms and conditions of a written
--  license agreement between you, or your company, and Synopsys, Inc.
--
--  The entire notice above must be reproduced on all authorized copies.
--
-- File :                       DW_ahb.v
-- Date :                       $Date: 2004/06/09 14:24:40 $
-- Version      :               $Revision: 1.61 $
-- Abstract     :               Top-level DW_ahb BusIP
--
------------------------------------------------------------------------
*/

`include "DW_amba_constants.v"
`include "DW_ahb_constants.v"
`include "DW_ahb_cc_constants.v"


module DW_ahb (
  hclk,
               hresetn,
               pause,
               haddr_m1,
               hburst_m1,
               hbusreq_m1,
               hlock_m1,
               hprot_m1,
               hsize_m1,
               htrans_m1,
               hwdata_m1,
               hwrite_m1,
               hgrant_m1,
               haddr_m2,
               hburst_m2,
               hbusreq_m2,
               hlock_m2,
               hprot_m2,
               hsize_m2,
               htrans_m2,
               hwdata_m2,
               hwrite_m2,
               hgrant_m2,
               hsel_s0,
               hready_resp_s0,
               hresp_s0,
               hrdata_s0,
               hsel_s1,
               hready_resp_s1,
               hresp_s1,
               hrdata_s1,
               hsel_s2,
               hready_resp_s2,
               hresp_s2,
               hrdata_s2,
               haddr,
               hburst,
               hprot,
               hsize,
               htrans,
               hwdata,
               hwrite,
               hready,
               hresp,
               hrdata,
               ahbarbint,
               hmaster,
               hmaster_data,
               hmastlock
               );

  // ----------------------------------------
  // Parameters inherited from the cc_constant 
  // file, make them not visible from the GUI
  // ----------------------------------------

  // physical parameters
  parameter haddr_width = 32;
  parameter ahb_data_width = 32;
  parameter big_endian = 0;

  // memory map parameters
  parameter r1_n_sa_1 = 32'h2000000;
  parameter r1_n_ea_1 = 32'h200ffff;
  parameter r1_n_sa_2 = 32'ha000000;
  parameter r1_n_ea_2 = 32'ha00ffff;

  // derived parameters
  parameter addrbus_width = 96;
  parameter databus_width = 96;
  parameter hrdatabus_width = 96;

  input                          hclk;
  input                          hresetn;
  input                          pause;

// Master #1 AHB signals
  input [haddr_width-1:0]        haddr_m1;
  input                          hbusreq_m1;
  input [`HBURST_WIDTH-1:0]      hburst_m1;
  input                          hlock_m1;
  input [`HPROT_WIDTH-1:0]       hprot_m1;
  input [`HSIZE_WIDTH-1:0]       hsize_m1;
  input [`HTRANS_WIDTH-1:0]      htrans_m1;
  input [ahb_data_width-1:0]     hwdata_m1;
  input                          hwrite_m1;
  output                         hgrant_m1;

// Master #2 AHB signals
  input [haddr_width-1:0]        haddr_m2;
  input                          hbusreq_m2;
  input [`HBURST_WIDTH-1:0]      hburst_m2;
  input                          hlock_m2;
  input [`HPROT_WIDTH-1:0]       hprot_m2;
  input [`HSIZE_WIDTH-1:0]       hsize_m2;
  input [`HTRANS_WIDTH-1:0]      htrans_m2;
  input [ahb_data_width-1:0]     hwdata_m2;
  input                          hwrite_m2;
  output                         hgrant_m2;
// Slave Arbiter AHB signals
  output                         hsel_s0;
  output                         hready_resp_s0;
  output [`HRESP_WIDTH-1:0]      hresp_s0;
  output [ahb_data_width-1:0]    hrdata_s0;

// Slave #1 AHB signals
  input                          hready_resp_s1;
  input [`HRESP_WIDTH-1:0]       hresp_s1;
  input [ahb_data_width-1:0]     hrdata_s1;
  output                         hsel_s1;

// Slave #2 AHB signals
  input                          hready_resp_s2;
  input [`HRESP_WIDTH-1:0]       hresp_s2;
  input [ahb_data_width-1:0]     hrdata_s2;
  output                         hsel_s2;
  output [haddr_width-1:0]      haddr;
  output [`HBURST_WIDTH-1:0]     hburst;
  output [`HPROT_WIDTH-1:0]      hprot;
  output [`HSIZE_WIDTH-1:0]      hsize;
  output [`HTRANS_WIDTH-1:0]     htrans;
  output [ahb_data_width-1:0]   hwdata;
  output                         hwrite;
  output [`HMASTER_WIDTH-1:0]    hmaster;
  output [`HMASTER_WIDTH-1:0]    hmaster_data;
  output                         hmastlock;

// Interrupt from early burst termination
  output                         ahbarbint;
  output                         hready;
  output [`HRESP_WIDTH-1:0]      hresp;
  output [ahb_data_width-1:0]    hrdata;


// Dummy wire declarations for inputs which may have been removed
// by reuse pragmas.

  wire                           ahb_big_endian;
  wire                           remap_n;
  wire                           pause;
  wire                           ahb_sc_arb;
  wire                           ahbarbint;

// Internal concatenated bus of all top-level haddr buses from masters
  wire   [addrbus_width-1:0]     bus_haddr;
  wire   [`HTRANSBUS_WIDTH-1:0]  bus_htrans;
  wire   [`NUM_IAHB_SLAVES:0]    bus_hsel;
  wire   [`INTERNAL_HSEL-1:0]    hsel;
  wire   [`SPLITBUS_WIDTH-1:0]   bus_hsplit;
  wire   [`HRESPBUS_WIDTH-1:0]   bus_hresp;
  wire   [`HREADY_WIDTH-1:0]     bus_hready;
  wire   [hrdatabus_width-1:0]   bus_hrdata;
  wire   [`HBURSTBUS_WIDTH-1:0]  bus_hburst;
  wire   [`HSIZEBUS_WIDTH-1:0]   bus_hsize;
  wire   [`HPROTBUS_WIDTH-1:0]   bus_hprot;
  wire   [`HWRITEBUS_WIDTH-1:0]  bus_hwrite;
  wire   [databus_width-1:0]     bus_hwdata;

  wire   [`HMASTER_WIDTH-1:0]    hmaster_data;
  wire                           hsel_s0;
  wire                           hready_resp_s0;
  wire   [ahb_data_width-1:0]    hrdata_s0;
  wire   [`HRESP_WIDTH-1:0]      hresp_s0;
  wire   [`NUM_AHB_MASTERS:0]    bus_hbusreq;
  wire   [`NUM_AHB_MASTERS:0]    bus_hlock;
  wire   [`NUM_AHB_MASTERS:0]    bus_hgrant;
  wire                           hsel_none;
  wire   [`HRESP_WIDTH-1:0]      hresp_none;
  wire                           hready_resp_none;
  wire   [ahb_data_width-1:0]    hrdata_none;
  wire                           int_pause;
  wire                           int_ahb_sc_arb;
  wire                           int_remap_n;
  wire                           int_ahb_big_endian;
  wire   [`NUM_AHB_MASTERS:1]    ahb_wt_mask;
  wire                           ahb_wt_aps;

  wire [`AHB_CCL_WIDTH-1:0]      ahb_wt_count_m15;
  wire [`AHB_CCL_WIDTH-1:0]      ahb_wt_count_m14;
  wire [`AHB_CCL_WIDTH-1:0]      ahb_wt_count_m13;
  wire [`AHB_CCL_WIDTH-1:0]      ahb_wt_count_m12;
  wire [`AHB_CCL_WIDTH-1:0]      ahb_wt_count_m11;
  wire [`AHB_CCL_WIDTH-1:0]      ahb_wt_count_m10;
  wire [`AHB_CCL_WIDTH-1:0]      ahb_wt_count_m9;
  wire [`AHB_CCL_WIDTH-1:0]      ahb_wt_count_m8;
  wire [`AHB_CCL_WIDTH-1:0]      ahb_wt_count_m7;
  wire [`AHB_CCL_WIDTH-1:0]      ahb_wt_count_m6;
  wire [`AHB_CCL_WIDTH-1:0]      ahb_wt_count_m5;
  wire [`AHB_CCL_WIDTH-1:0]      ahb_wt_count_m4;
  wire [`AHB_CCL_WIDTH-1:0]      ahb_wt_count_m3;
  wire [`AHB_CCL_WIDTH-1:0]      ahb_wt_count_m2;
  wire [`AHB_CCL_WIDTH-1:0]      ahb_wt_count_m1;

// These are the signals driven by Master 0, the Internal Dummy
// Master
// The Dummy Master is granted the bus when no other master
// can access the bus.
// The function of the Dummy Master is to drive Default
// Values onto the Address, Data and Control buses when
// no other master can gain access to the bus.

//
// The dummy master
//

  assign bus_haddr[haddr_width-1:0]     = {haddr_width{1'b0}};
  assign bus_htrans[`HTRANS_WIDTH-1:0]   = `IDLE;
  assign bus_hwdata[ahb_data_width-1:0] = {ahb_data_width{1'b0}};
  assign bus_hsize[`HSIZE_WIDTH-1:0]     = `BYTE;
  assign bus_hburst[`HBURST_WIDTH-1:0]   = `SINGLE;
  assign bus_hprot[`HPROT_WIDTH-1:0]     = `NC_NB_P_D;
  assign bus_hwrite[0]                   = `READ;

//
// Build internal busses from master and slave signals, The following 
// assign statements are generated by a tcl plugin script.
//
   
  assign bus_haddr[(haddr_width*2)-1:(haddr_width*1)] = haddr_m1;
  assign bus_htrans[3:2] = htrans_m1;
  assign bus_hburst[5:3] = hburst_m1;
  assign bus_hsize[5:3] = hsize_m1;
  assign bus_hprot[7:4] = hprot_m1;
  assign bus_hwrite[1] = hwrite_m1;
  assign bus_hwdata[(ahb_data_width*2)-1:(ahb_data_width*1)] = hwdata_m1;

  assign bus_hbusreq[1] = hbusreq_m1;
  assign hgrant_m1 = bus_hgrant[1];
  assign bus_hlock[1] = hlock_m1;

  assign bus_haddr[(haddr_width*3)-1:(haddr_width*2)] = haddr_m2;
  assign bus_htrans[5:4] = htrans_m2;
  assign bus_hburst[8:6] = hburst_m2;
  assign bus_hsize[8:6] = hsize_m2;
  assign bus_hprot[11:8] = hprot_m2;
  assign bus_hwrite[2] = hwrite_m2;
  assign bus_hwdata[(ahb_data_width*3)-1:(ahb_data_width*2)] = hwdata_m2;

  assign bus_hbusreq[2] = hbusreq_m2;
  assign hgrant_m2 = bus_hgrant[2];
  assign bus_hlock[2] = hlock_m2;

  assign bus_hbusreq[0] = 1'b0;
  assign bus_hlock[0] = 1'b0;
  assign bus_hready[0] = hready_resp_s0;
  assign bus_hresp[1:0] = hresp_s0;
  assign bus_hrdata[ahb_data_width-1:0] = hrdata_s0;

  assign bus_hready[1] = hready_resp_s1;
  assign bus_hresp[3:2] = hresp_s1;
  assign bus_hrdata[(ahb_data_width*2)-1:ahb_data_width*1] = hrdata_s1;

  assign bus_hready[2] = hready_resp_s2;
  assign bus_hresp[5:4] = hresp_s2;
  assign bus_hrdata[(ahb_data_width*3)-1:ahb_data_width*2] = hrdata_s2;

  assign hsel_none = hsel[`NUM_IAHB_SLAVES+1];
  assign hsel_s0 = hsel[0];
  assign hsel_s1 = hsel[1];
  assign hsel_s2 = hsel[2];
  assign bus_hsel = hsel[`NUM_IAHB_SLAVES:0];

  assign bus_hsplit[15:0] = {`HSPLIT_WIDTH{1'b0}};
  assign bus_hsplit[31:16] = {`HSPLIT_WIDTH{1'b0}};


// end of generated "assign" statements
   
  DW_ahb_mux
   #(haddr_width, ahb_data_width)
   U_mux (
    .hclk             (hclk),
    .hresetn          (hresetn),
    .bus_haddr        (bus_haddr),
    .bus_hburst       (bus_hburst),
    .hmaster          (hmaster),
    .bus_hprot        (bus_hprot),
    .bus_hsize        (bus_hsize),
    .bus_htrans       (bus_htrans),
    .bus_hwdata       (bus_hwdata),
    .bus_hwrite       (bus_hwrite),
    .hrdata_none      (hrdata_none),
    .hready_resp_none (hready_resp_none),
    .hresp_none       (hresp_none),
    .bus_hready       (bus_hready),
    .bus_hresp        (bus_hresp),
    .bus_hrdata       (bus_hrdata),
    .hsel             (hsel[`NUM_IAHB_SLAVES:0]),
    .hmaster_data     (hmaster_data),
    .haddr            (haddr),
    .hburst           (hburst),
    .hprot            (hprot),
    .hsize            (hsize),
    .htrans           (htrans),
    .hwdata           (hwdata),
    .hwrite           (hwrite),
    .hrdata           (hrdata),
    .hready           (hready),
    .hresp            (hresp)
  );

// To avoid reuse pragmas swap use of port when it is configured to be
// removed.
  assign int_remap_n        = (`REMAP == 1'b1) ? remap_n : 1'b1;
  assign int_ahb_big_endian = (`AHB_XENDIAN == 1'b1) ? ahb_big_endian : 1'b1;
  
  DW_ahb_dcdr
   #(haddr_width, r1_n_sa_1, r1_n_ea_1, r1_n_sa_2, r1_n_ea_2) U_dcdr (
    .haddr            (haddr),
    .remap_n          (int_remap_n),
    .hsel             (hsel)
  );


  assign int_pause      = (`PAUSE == 1'b1) ? pause : 1'b0;
  assign int_ahb_sc_arb = (`AHB_SINGLE_CYCLE_ARBITRATION == 1'b1) ? ahb_sc_arb : 1'b0;

  DW_ahb_arb
   #(haddr_width, ahb_data_width, big_endian) U_arb (
    .hclk             (hclk),
    .hresetn          (hresetn),
    .ahb_sc_arb       (int_ahb_sc_arb),
    .hready           (hready),
    .hresp            (hresp),
    .hsel             (hsel_s0),
    .haddr            (haddr),
    .hburst           (hburst),
    .hsize            (hsize),
    .htrans           (htrans),
    .hwdata           (hwdata),
    .hwrite           (hwrite),
    .bus_hlock        (bus_hlock),
    .bus_hbusreq      (bus_hbusreq),
    .bus_hsplit       (bus_hsplit),
    .hmaster_data     (hmaster_data),
    .pause            (int_pause),
    .ahb_big_endian   (int_ahb_big_endian),
    
    .bus_hgrant       (bus_hgrant),
    .ahbarbint        (ahbarbint),
    .hready_resp_s0   (hready_resp_s0),
    .hresp_s0         (hresp_s0),
    .hrdata_s0        (hrdata_s0),
    .hmaster          (hmaster),
    .hmastlock        (hmastlock),
    .wt_count_m15     (ahb_wt_count_m15),
    .wt_count_m14     (ahb_wt_count_m14),
    .wt_count_m13     (ahb_wt_count_m13),
    .wt_count_m12     (ahb_wt_count_m12),
    .wt_count_m11     (ahb_wt_count_m11),
    .wt_count_m10     (ahb_wt_count_m10),
    .wt_count_m9      (ahb_wt_count_m9),
    .wt_count_m8      (ahb_wt_count_m8),
    .wt_count_m7      (ahb_wt_count_m7),
    .wt_count_m6      (ahb_wt_count_m6),
    .wt_count_m5      (ahb_wt_count_m5),
    .wt_count_m4      (ahb_wt_count_m4),
    .wt_count_m3      (ahb_wt_count_m3),
    .wt_count_m2      (ahb_wt_count_m2),
    .wt_count_m1      (ahb_wt_count_m1),
    .ahb_wt_mask      (ahb_wt_mask),
    .ahb_wt_aps       (ahb_wt_aps)
  );




  DW_ahb_dfltslv
   #(ahb_data_width)
   U_dfltslv (
    .hclk             (hclk),
    .hresetn          (hresetn),
    .hready           (hready),
    .htrans           (htrans),
    .hsel_none        (hsel_none),
    .hready_resp_none (hready_resp_none),
    .hresp_none       (hresp_none),
    .hrdata_none      (hrdata_none)
  );

endmodule
