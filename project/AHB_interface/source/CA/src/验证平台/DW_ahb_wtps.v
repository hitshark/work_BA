/*
------------------------------------------------------------------------
--
--                  (C) COPYRIGHT 2001-2004 SYNOPSYS, INC.
--                             ALL RIGHTS RESERVED
--
--  This software and the associated documentation are confidential and
--  proprietary to Synopsys, Inc.  Your use or disclosure of this
--  software is subject to the terms and conditions of a written
--  license agreement between you, or your company, and Synopsys, Inc.
--
--  The entire notice above must be reproduced on all authorized copies.
--
-- File :                       DW_ahb_wtps.v
-- Author:                      Peter Gillen 
-- Date :                       $Date: 2004/06/09 14:24:45 $ 
-- Version      :               $Revision: 1.6 $ 
-- Abstract     :
--
-- Each master has a counter which counts the number of clock tokens each
-- master uses up in an arbitration period. When the tokens are used then
-- this master can no longer compete at the upper level. When masters are
-- masked from this upper level they can gain access when no one else is
-- looking for the bus who has any spare tokens. When all tokens are used
-- then normal arbitration continues. This requires the weighted token
-- arbitration scheme to be enabled, otherwise the default arbitration
-- scheme is in place.
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

module DW_ahb_wtps (
  hclk,
  hresetn,
  hready,
  hmaster,
  pause,
  wten,
  ahb_itcl,
  bus_ahb_icl,
  mask,

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

  tokenmask,
  ahb_wt_aps
);

input                          hclk;
input                          hresetn;
input                          hready;
input                          pause;
// Configuration: Weighted token enable
input                          wten;
input [`NUM_AHB_MASTERS:0]     mask;
input [`HMASTER_WIDTH-1:0]     hmaster;
// Configuration: the number of clock tokens in arbitration period
input [`AHB_TCL_WIDTH-1:0]     ahb_itcl;
// Configuration of all the master clock tokens for each counter
input [`BUS_AHB_CCL_WIDTH-1:0] bus_ahb_icl;

// When a master runs out of tokens it is to be masked from the upper arbitration
// scheme using the tokenmask bit.

output [`AHB_CCL_WIDTH-1:0]      wt_count_m15;
output [`AHB_CCL_WIDTH-1:0]      wt_count_m14;
output [`AHB_CCL_WIDTH-1:0]      wt_count_m13;
output [`AHB_CCL_WIDTH-1:0]      wt_count_m12;
output [`AHB_CCL_WIDTH-1:0]      wt_count_m11;
output [`AHB_CCL_WIDTH-1:0]      wt_count_m10;
output [`AHB_CCL_WIDTH-1:0]      wt_count_m9;
output [`AHB_CCL_WIDTH-1:0]      wt_count_m8;
output [`AHB_CCL_WIDTH-1:0]      wt_count_m7;
output [`AHB_CCL_WIDTH-1:0]      wt_count_m6;
output [`AHB_CCL_WIDTH-1:0]      wt_count_m5;
output [`AHB_CCL_WIDTH-1:0]      wt_count_m4;
output [`AHB_CCL_WIDTH-1:0]      wt_count_m3;
output [`AHB_CCL_WIDTH-1:0]      wt_count_m2;
output [`AHB_CCL_WIDTH-1:0]      wt_count_m1;
output [`NUM_AHB_MASTERS:0]    tokenmask;
output                         ahb_wt_aps;

reg    [`AHB_TCL_WIDTH-1:0]    ahb_tccnt;
wire   [15:0]                  wt_mask;

wire [`AHB_CCL_WIDTH-1:0]        iwt_count_m15;
wire [`AHB_CCL_WIDTH-1:0]        iwt_count_m14;
wire [`AHB_CCL_WIDTH-1:0]        iwt_count_m13;
wire [`AHB_CCL_WIDTH-1:0]        iwt_count_m12;
wire [`AHB_CCL_WIDTH-1:0]        iwt_count_m11;
wire [`AHB_CCL_WIDTH-1:0]        iwt_count_m10;
wire [`AHB_CCL_WIDTH-1:0]        iwt_count_m9;
wire [`AHB_CCL_WIDTH-1:0]        iwt_count_m8;
wire [`AHB_CCL_WIDTH-1:0]        iwt_count_m7;
wire [`AHB_CCL_WIDTH-1:0]        iwt_count_m6;
wire [`AHB_CCL_WIDTH-1:0]        iwt_count_m5;
wire [`AHB_CCL_WIDTH-1:0]        iwt_count_m4;
wire [`AHB_CCL_WIDTH-1:0]        iwt_count_m3;
wire [`AHB_CCL_WIDTH-1:0]        iwt_count_m2;
wire [`AHB_CCL_WIDTH-1:0]        iwt_count_m1;

// Enables for the counters to count.
// en_tccnt       : The type of counting mode, clock or bus cycle
// en_ahb_tccnt   : Requires the mode to be enabled plus the counting mode
// en_ahb_ccnt_mx : When hmaster is x and en_ahb_tccnt is active then the
//                  master is using a clock token and the count is decremented

wire                           en_tccnt;
wire                           en_ahb_tccnt;
wire                           en_ahb_ccnt_m1;
wire                           en_ahb_ccnt_m2;
wire                           en_ahb_ccnt_m3;
wire                           en_ahb_ccnt_m4;
wire                           en_ahb_ccnt_m5;
wire                           en_ahb_ccnt_m6;
wire                           en_ahb_ccnt_m7;
wire                           en_ahb_ccnt_m8;
wire                           en_ahb_ccnt_m9;
wire                           en_ahb_ccnt_m10;
wire                           en_ahb_ccnt_m11;
wire                           en_ahb_ccnt_m12;
wire                           en_ahb_ccnt_m13;
wire                           en_ahb_ccnt_m14;
wire                           en_ahb_ccnt_m15;

// Each master generates a mask bit wt_mask_mx and this is overridden with
// iwt_mask_mx. The over riding is done for masters that are not included
// into the design. For 14 master system then master 15 mask are set to 0.

wire                           wt_mask_m1;
wire                           wt_mask_m2;
wire                           wt_mask_m3;
wire                           wt_mask_m4;
wire                           wt_mask_m5;
wire                           wt_mask_m6;
wire                           wt_mask_m7;
wire                           wt_mask_m8;
wire                           wt_mask_m9;
wire                           wt_mask_m10;
wire                           wt_mask_m11;
wire                           wt_mask_m12;
wire                           wt_mask_m13;
wire                           wt_mask_m14;
wire                           wt_mask_m15;
wire                           iwt_mask_m1;
wire                           iwt_mask_m2;
wire                           iwt_mask_m3;
wire                           iwt_mask_m4;
wire                           iwt_mask_m5;
wire                           iwt_mask_m6;
wire                           iwt_mask_m7;
wire                           iwt_mask_m8;
wire                           iwt_mask_m9;
wire                           iwt_mask_m10;
wire                           iwt_mask_m11;
wire                           iwt_mask_m12;
wire                           iwt_mask_m13;
wire                           iwt_mask_m14;
wire                           iwt_mask_m15;

// Arbitration period start
wire                        arb_pstart;
// When one returns from pause assuming it is implemented then the 
// arbitration period is restarted and the counters are reloaded.
// A falling edge detecor on pause is used to signal that the values
// should be reloaded, this is only used when the PAUSE mode
// functionality is enabled.
reg                         d_pause;
wire                        fed_pause;
wire                        qfed_pause;

// When the weighted token is enabled then the counters are loaded
// and a new arbitration period is started. The mode should be
// disabled when the counter values are being changed and then
// reenabled when the value are all programmed. A rising edge
// detector is used to load the counters when the mode is enabled.
wire                        red_wten;
reg                         d_wten;

assign ahb_wt_aps = arb_pstart;

//--------------------------------------------------------------------
// Rising Edge Detector on Weighted Token Arbitration
//--------------------------------------------------------------------
//
// d_wten : When the user programs the weighted token enable
// this change is used to load the counters for the arbitration
// period and for the clock tokens. Looking for a rising edge
// to load the counter. The counter is subsequently loaded when
// the timer loops around zero.
//


  always @ (posedge hclk or negedge hresetn)
  begin : d_wten_PROC
    if (hresetn == 1'b0)
      d_wten <= 1'b0;
    else
      d_wten <= wten;
  end 
  assign red_wten = (d_wten == 1'b0) && (wten == 1'b1);

//--------------------------------------------------------------------
// Falling Edge Detector on pause provided configured
//--------------------------------------------------------------------
//
// pause will be set when the arbiter enters pause mode
// When one comes out of pause mode then all the counters
// will be reloaded. If pause mode is not enabled then this
// functionality is bypassed.

  always @ (posedge hclk or negedge hresetn)
  begin : d_pause_PROC
    if (hresetn == 1'b0)
      d_pause <= 1'b0;
    else
      d_pause <= pause;
  end 
  assign fed_pause = (d_pause == 1'b1) && (pause == 1'b0);
  assign qfed_pause = (`PAUSE == 0) ? 1'b0 : fed_pause;

//--------------------------------------------------------------------
// Arbitration Period Start
//--------------------------------------------------------------------
//
// The counter is loaded whenever the count is zero
// The counter is loaded whenever one comes out of pause mode, as one
// could have entered pause mode in the middle of an arbitration period
// The counter is loaded when the weighted token arbitration scheme is
// enabled.
//
  assign arb_pstart = (ahb_tccnt == {`AHB_TCL_WIDTH{1'b0}}) || 
                      (fed_pause == 1'b1) || 
                      (red_wten  == 1'b1);

//--------------------------------------------------------------------
// Arbitration Enabling
//--------------------------------------------------------------------
//
// When one has AHB_TPS_MODE as clock cycle mode then the counter
// is always enabled. When one has AHB_TPS_MODE as bus cycle mode
// the counter is enabled when hready is active. In  both cases one
// requires the weighted token enable to be set.
//
  assign en_tccnt     = (`AHB_TPS_MODE == 1'b0) ? 1'b1 : hready;
  assign en_ahb_tccnt = wten && en_tccnt;

//--------------------------------------------------------------------
// Arbitration Period Counter
//--------------------------------------------------------------------
//
// ahb_tccnt : AHB Total Cycle Count
// Loaded with ahb_itcl when count reaches zero
// Decremented by 1 on each enable cycle
//
  always @ (posedge hclk or negedge hresetn)
  begin : ahb_tccnt_PROC
    if (hresetn == 1'b0)
      ahb_tccnt <= {`AHB_TCL_WIDTH{1'b0}};
    else begin
      if (arb_pstart == 1'b1) begin
        ahb_tccnt <= ahb_itcl;
      end else begin
        if (en_ahb_tccnt == 1'b1) begin
          ahb_tccnt <= ahb_tccnt - 1;
        end
      end
    end
  end

//--------------------------------------------------------------------
// Master Clock Token Counter 
//--------------------------------------------------------------------
//
// ahb_ccnt_m1 : AHB Cycle Count Master 1
// Loaded with ahb_icl_m1 when the arbitration period starts, this is
// part of the concatenated bus bus_ahb_icl which is made up of each
// masters load value.
// Decremented by 1 on each enable cycle provided the bus owner
// is master number 1. Reset with macro AHB_CL_M1.
//


  assign en_ahb_ccnt_m1 = ((hmaster == 4'd1) && (en_ahb_tccnt == 1'b1));
  assign iwt_mask_m1 = (`AHB_WTEN == 1) ? wt_mask_m1 : 1'b0;
  assign wt_count_m1 = (`AHB_WTEN == 1) ? iwt_count_m1 : {`AHB_CCL_WIDTH{1'b0}};

//
// ahb_ccnt_m2 : AHB Cycle Count Master 2
// Loaded with ahb_icl_m1 when the arbitration period starts, this is
// part of the concatenated bus bus_ahb_icl which is made up of each
// masters load value.
// Decremented by 1 on each enable cycle provided the bus owner
// is master number 2. Reset with macro AHB_CL_M2.
//

  assign en_ahb_ccnt_m2 = ((hmaster == 4'd2) && (en_ahb_tccnt == 1'b1));
  assign iwt_mask_m2 = ((`AHB_WTEN == 1) && (`NUM_AHB_MASTERS > 1)) ? wt_mask_m2 : 1'b0;
  assign wt_count_m2 = ((`AHB_WTEN == 1) && (`NUM_AHB_MASTERS > 1)) ? iwt_count_m2 : {`AHB_CCL_WIDTH{1'b0}};

//
// ahb_ccnt_m3 : AHB Cycle Count Master 3
// Loaded with ahb_icl_m1 when the arbitration period starts, this is
// part of the concatenated bus bus_ahb_icl which is made up of each
// masters load value.
// Decremented by 1 on each enable cycle provided the bus owner
// is master number 3. Reset with macro AHB_CL_M3.
//

  assign en_ahb_ccnt_m3 = ((hmaster == 4'd3) && (en_ahb_tccnt == 1'b1));
  assign iwt_mask_m3 = ((`AHB_WTEN == 1) && (`NUM_AHB_MASTERS > 2)) ? wt_mask_m3 : 1'b0;
  assign wt_count_m3 = ((`AHB_WTEN == 1) && (`NUM_AHB_MASTERS > 2)) ? iwt_count_m3 : {`AHB_CCL_WIDTH{1'b0}};

//
// ahb_ccnt_m4 : AHB Cycle Count Master 4
// Loaded with ahb_icl_m1 when the arbitration period starts, this is
// part of the concatenated bus bus_ahb_icl which is made up of each
// masters load value.
// Decremented by 1 on each enable cycle provided the bus owner
// is master number 4. Reset with macro AHB_CL_M4.
//

  assign en_ahb_ccnt_m4 = ((hmaster == 4'd4) && (en_ahb_tccnt == 1'b1));
  assign iwt_mask_m4 = ((`AHB_WTEN == 1) && (`NUM_AHB_MASTERS > 3)) ? wt_mask_m4 : 1'b0;
  assign wt_count_m4 = ((`AHB_WTEN == 1) && (`NUM_AHB_MASTERS > 3)) ? iwt_count_m4 : {`AHB_CCL_WIDTH{1'b0}};

//
// ahb_ccnt_m5 : AHB Cycle Count Master 5
// Loaded with ahb_icl_m1 when the arbitration period starts, this is
// part of the concatenated bus bus_ahb_icl which is made up of each
// masters load value.
// Decremented by 1 on each enable cycle provided the bus owner
// is master number 5. Reset with macro AHB_CL_M5.
//

  assign en_ahb_ccnt_m5 = ((hmaster == 4'd5) && (en_ahb_tccnt == 1'b1));
  assign iwt_mask_m5 = ((`AHB_WTEN == 1) && (`NUM_AHB_MASTERS > 4)) ? wt_mask_m5 : 1'b0;
  assign wt_count_m5 = ((`AHB_WTEN == 1) && (`NUM_AHB_MASTERS > 4)) ? iwt_count_m5 : {`AHB_CCL_WIDTH{1'b0}};

//
// ahb_ccnt_m6 : AHB Cycle Count Master 6
// Loaded with ahb_icl_m1 when the arbitration period starts, this is
// part of the concatenated bus bus_ahb_icl which is made up of each
// masters load value.
// Decremented by 1 on each enable cycle provided the bus owner
// is master number 6. Reset with macro AHB_CL_M6.
//

  assign en_ahb_ccnt_m6 = ((hmaster == 4'd6) && (en_ahb_tccnt == 1'b1));
  assign iwt_mask_m6 = ((`AHB_WTEN == 1) && (`NUM_AHB_MASTERS > 5)) ? wt_mask_m6 : 1'b0;
  assign wt_count_m6 = ((`AHB_WTEN == 1) && (`NUM_AHB_MASTERS > 5)) ? iwt_count_m6 : {`AHB_CCL_WIDTH{1'b0}};

//
// ahb_ccnt_m7 : AHB Cycle Count Master 7
// Loaded with ahb_icl_m1 when the arbitration period starts, this is
// part of the concatenated bus bus_ahb_icl which is made up of each
// masters load value.
// Decremented by 1 on each enable cycle provided the bus owner
// is master number 7. Reset with macro AHB_CL_M7.
//

  assign en_ahb_ccnt_m7 = ((hmaster == 4'd7) && (en_ahb_tccnt == 1'b1));
  assign iwt_mask_m7 = ((`AHB_WTEN == 1) && (`NUM_AHB_MASTERS > 6)) ? wt_mask_m7 : 1'b0;
  assign wt_count_m7 = ((`AHB_WTEN == 1) && (`NUM_AHB_MASTERS > 6)) ? iwt_count_m7 : {`AHB_CCL_WIDTH{1'b0}};

//
// ahb_ccnt_m8 : AHB Cycle Count Master 8
// Loaded with ahb_icl_m1 when the arbitration period starts, this is
// part of the concatenated bus bus_ahb_icl which is made up of each
// masters load value.
// Decremented by 1 on each enable cycle provided the bus owner
// is master number 8. Reset with macro AHB_CL_M8.
//

  assign en_ahb_ccnt_m8 = ((hmaster == 4'd8) && (en_ahb_tccnt == 1'b1));
  assign iwt_mask_m8 = ((`AHB_WTEN == 1) && (`NUM_AHB_MASTERS > 7)) ? wt_mask_m8 : 1'b0;
  assign wt_count_m8 = ((`AHB_WTEN == 1) && (`NUM_AHB_MASTERS > 7)) ? iwt_count_m8 : {`AHB_CCL_WIDTH{1'b0}};

//
// ahb_ccnt_m9 : AHB Cycle Count Master 9
// Loaded with ahb_icl_m1 when the arbitration period starts, this is
// part of the concatenated bus bus_ahb_icl which is made up of each
// masters load value.
// Decremented by 1 on each enable cycle provided the bus owner
// is master number 9. Reset with macro AHB_CL_M9.
//

  assign en_ahb_ccnt_m9 = ((hmaster == 4'd9) && (en_ahb_tccnt == 1'b1));
  assign iwt_mask_m9 = ((`AHB_WTEN == 1) && (`NUM_AHB_MASTERS > 8)) ? wt_mask_m9 : 1'b0;
  assign wt_count_m9 = ((`AHB_WTEN == 1) && (`NUM_AHB_MASTERS > 8)) ? iwt_count_m9 : {`AHB_CCL_WIDTH{1'b0}};

//
// ahb_ccnt_m10 : AHB Cycle Count Master 10
// Loaded with ahb_icl_m1 when the arbitration period starts, this is
// part of the concatenated bus bus_ahb_icl which is made up of each
// masters load value.
// Decremented by 1 on each enable cycle provided the bus owner
// is master number 10. Reset with macro AHB_CL_M10.
//

  assign en_ahb_ccnt_m10 = ((hmaster == 4'd10) && (en_ahb_tccnt == 1'b1));
  assign iwt_mask_m10 = ((`AHB_WTEN == 1) && (`NUM_AHB_MASTERS > 9)) ? wt_mask_m10 : 1'b0;
  assign wt_count_m10 = ((`AHB_WTEN == 1) && (`NUM_AHB_MASTERS > 9)) ? iwt_count_m10 : {`AHB_CCL_WIDTH{1'b0}};

//
// ahb_ccnt_m11 : AHB Cycle Count Master 11
// Loaded with ahb_icl_m1 when the arbitration period starts, this is
// part of the concatenated bus bus_ahb_icl which is made up of each
// masters load value.
// Decremented by 1 on each enable cycle provided the bus owner
// is master number 11. Reset with macro AHB_CL_M11.
//

  assign en_ahb_ccnt_m11 = ((hmaster == 4'd11) && (en_ahb_tccnt == 1'b1));
  assign iwt_mask_m11 = ((`AHB_WTEN == 1) && (`NUM_AHB_MASTERS > 10)) ? wt_mask_m11 : 1'b0;
  assign wt_count_m11 = ((`AHB_WTEN == 1) && (`NUM_AHB_MASTERS > 10)) ? iwt_count_m11 : {`AHB_CCL_WIDTH{1'b0}};

//
// ahb_ccnt_m12 : AHB Cycle Count Master 12
// Loaded with ahb_icl_m1 when the arbitration period starts, this is
// part of the concatenated bus bus_ahb_icl which is made up of each
// masters load value.
// Decremented by 1 on each enable cycle provided the bus owner
// is master number 12. Reset with macro AHB_CL_M12.
//

  assign en_ahb_ccnt_m12 = ((hmaster == 4'd12) && (en_ahb_tccnt == 1'b1));
  assign iwt_mask_m12 = ((`AHB_WTEN == 1) && (`NUM_AHB_MASTERS > 11)) ? wt_mask_m12 : 1'b0;
  assign wt_count_m12 = ((`AHB_WTEN == 1) && (`NUM_AHB_MASTERS > 11)) ? iwt_count_m12 : {`AHB_CCL_WIDTH{1'b0}};

//
// ahb_ccnt_m13 : AHB Cycle Count Master 13
// Loaded with ahb_icl_m1 when the arbitration period starts, this is
// part of the concatenated bus bus_ahb_icl which is made up of each
// masters load value.
// Decremented by 1 on each enable cycle provided the bus owner
// is master number 13. Reset with macro AHB_CL_M13.
//

  assign en_ahb_ccnt_m13 = ((hmaster == 4'd13) && (en_ahb_tccnt == 1'b1));
  assign iwt_mask_m13 = ((`AHB_WTEN == 1) && (`NUM_AHB_MASTERS > 12)) ? wt_mask_m13 : 1'b0;
  assign wt_count_m13 = ((`AHB_WTEN == 1) && (`NUM_AHB_MASTERS > 12)) ? iwt_count_m13 : {`AHB_CCL_WIDTH{1'b0}};

//
// ahb_ccnt_m14 : AHB Cycle Count Master 14
// Loaded with ahb_icl_m1 when the arbitration period starts, this is
// part of the concatenated bus bus_ahb_icl which is made up of each
// masters load value.
// Decremented by 1 on each enable cycle provided the bus owner
// is master number 14. Reset with macro AHB_CL_M14.
//

  assign en_ahb_ccnt_m14 = ((hmaster == 4'd14) && (en_ahb_tccnt == 1'b1));
  assign iwt_mask_m14 = ((`AHB_WTEN == 1) && (`NUM_AHB_MASTERS > 13)) ? wt_mask_m14 : 1'b0;
  assign wt_count_m14 = ((`AHB_WTEN == 1) && (`NUM_AHB_MASTERS > 13)) ? iwt_count_m14 : {`AHB_CCL_WIDTH{1'b0}};

//
// ahb_ccnt_m15 : AHB Cycle Count Master 15
// Loaded with ahb_icl_m1 when the arbitration period starts, this is
// part of the concatenated bus bus_ahb_icl which is made up of each
// masters load value.
// Decremented by 1 on each enable cycle provided the bus owner
// is master number 15. Reset with macro AHB_CL_M15.
//

  assign en_ahb_ccnt_m15 = ((hmaster == 4'd15) && (en_ahb_tccnt == 1'b1));
  assign iwt_mask_m15  = ((`AHB_WTEN == 1) && (`NUM_AHB_MASTERS > 14)) ? wt_mask_m15 : 1'b0;
  assign wt_count_m15 = ((`AHB_WTEN == 1) && (`NUM_AHB_MASTERS > 14)) ? iwt_count_m15 : {`AHB_CCL_WIDTH{1'b0}};

//
// All mask bits are qualified and are concatenated, the relevant number of bits
// are then stripped depending on the number of masters in the system.
// Externally this is used with the output from DW_ahb_mask to remove a master from
// the upper arbitration scheme.
//
  assign wt_mask = { iwt_mask_m15, iwt_mask_m14, iwt_mask_m13, iwt_mask_m12,
                     iwt_mask_m11, iwt_mask_m10, iwt_mask_m9,  iwt_mask_m8,
                     iwt_mask_m7,  iwt_mask_m6,  iwt_mask_m5,  iwt_mask_m4,
                     iwt_mask_m3,  iwt_mask_m2,  iwt_mask_m1,  1'b0 };

  assign tokenmask = wt_mask[`NUM_AHB_MASTERS:0] | mask;

endmodule

