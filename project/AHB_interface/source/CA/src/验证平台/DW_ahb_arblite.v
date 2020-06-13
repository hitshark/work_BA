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
-- File :                       DW_ahb_arblite.v
-- Author:                      Peter Gillen 
-- Date :                       $Date: 2004/06/09 14:24:41 $ 
-- Version      :               $Revision: 1.5 $ 
-- Abstract     :               AHB Lite Arbiter Replacement
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


module DW_ahb_arblite (
  hclk,
  hresetn,
  hready,
  hlock_m1,
  hmaster,
  hmastlock
);

  input                        hclk;
  input                        hresetn;
  input                        hready;
  input                        hlock_m1;
  output [`HMASTER_WIDTH-1:0]  hmaster;
  output                       hmastlock;

  reg                          hmastlock;
  reg                          ihmaster;
//
// The timing of the hlock signal requires it to be retimed
// with hready so that the resultant hmastlock will have the
// same timing as the address and control on the AHB bus.
//
  always @(posedge hclk or negedge hresetn)
  begin : hmastlock_PROC
    if (hresetn == 1'b0) begin
      hmastlock <= 1'b0;
    end else begin
      if (hready == 1'b1) begin
        hmastlock <= hlock_m1;
      end
    end
  end

  assign hmaster = {4'b0001};

endmodule
