/*
------------------------------------------------------------------------
--
--                    (C) COPYRIGHT 2004 SYNOPSYS, INC.
--                           ALL RIGHTS RESERVED
--
--  This software and the associated documentation are confidential and
--  proprietary to Synopsys, Inc.  Your use or disclosure of this
--  software is subject to the terms and conditions of a written
--  license agreement between you, or your company, and Synopsys, Inc.
--
--  The entire notice above must be reproduced on all authorized copies.
--
-- File :                       DW_ahb_constants.v
-- Author:                      Ray Beechinor 
-- Date :                       $Date: 2004/06/09 14:24:42 $ 
-- Version      :               $Revision: 1.17 $ 
-- Abstract     :               Auxiliary Parameter File for DW_ahb
--                              and associated files.  Contains
--                              parameters used in DW_ahb 
--                              which are not
--                              directly configured by the user 
--                              in coreConsultant
--
-- Modification History:
-- Date                 By      Version Change  Description
-- =====================================================================
-- See CVS log
-- =====================================================================
*/


//`include "DW_amba_constants.v"
//`include "DW_ahb_constants.v"
//`include "DW_ahb_cc_constants.v"


// Description:  Visibility Control of Arbiter Slave
// DefaultValue: 3
// MinValue:     -2147483648
// MaxValue:     2147483647
`define VISIBLE_0 3


// Description:  Generate Only Hsel for Arbiter Slave
// DefaultValue: 0
// MinValue:     -2147483648
// MaxValue:     2147483647
`define HSEL_ONLY_S0 0


// Description:  Slave Alias for Arbiter
// DefaultValue: 0
// MinValue:     -2147483648
// MaxValue:     2147483647
`define ALIAS_S0 0


// Description:  Is Arbiter Split-capable?
// DefaultValue: 0
// MinValue:     -2147483648
// MaxValue:     2147483647
`define SPLIT_CAPABLE_0 0


// Description:  Internal Master PortCount
// DefaultValue: 3
// MinValue:     2
// MaxValue:     16
`define NUM_INT_MASTERS 3


// Description:  Internal Master PortCount
// DefaultValue: 3
// MinValue:     2
// MaxValue:     31
`define NUM_INT_SLAVES 3


// Description:  Internal HTRANS Bus Width
// DefaultValue: 6
// MinValue:     2
// MaxValue:     62
`define HRESPBUS_WIDTH 6


// Description:  Internal HTRANS Bus Width
// DefaultValue: 6
// MinValue:     2
// MaxValue:     32
`define HTRANSBUS_WIDTH 6


// Description:  HSIZE Address Bus Width
// DefaultValue: 9
// MinValue:     3
// MaxValue:     48
`define HSIZEBUS_WIDTH 9


// Description:  HBURST Address Bus Width
// DefaultValue: 9
// MinValue:     3
// MaxValue:     48
`define HBURSTBUS_WIDTH 9


// Description:  HPROT Address Bus Width
// DefaultValue: 12
// MinValue:     4
// MaxValue:     96
`define HPROTBUS_WIDTH 12


// Description:  HWRITE Address Bus Width
// DefaultValue: 3
// MinValue:     1
// MaxValue:     16
`define HWRITEBUS_WIDTH 3


// Description:  HWRITE Address Bus Width
// DefaultValue: 3
// MinValue:     1
// MaxValue:     32
`define HREADY_WIDTH 3


// Description:  InternalBusWidth
// DefaultValue: 4
// MinValue:     -2147483648
// MaxValue:     2147483647
`define INTERNAL_HSEL 4


// Description:  HWRITE Address Bus Width
// DefaultValue: 32
// MinValue:     -2147483648
// MaxValue:     2147483647
`define SPLITBUS_WIDTH 32


// Description:  Bussed Counter Width
// DefaultValue: 480
// MinValue:     -2147483648
// MaxValue:     2147483647
`define BUS_AHB_CCL_WIDTH 480

`define NC_NB_NP_O  4'b0000
`define NC_NB_NP_D  4'b0001
`define NC_NB_P_O   4'b0010
`define NC_NB_P_D   4'b0011

`define NC_B_NP_O   4'b0100
`define NC_B_NP_D   4'b0101
`define NC_B_P_O    4'b0110
`define NC_B_P_D    4'b0111

`define C_NB_NP_O   4'b1000
`define C_NB_NP_D   4'b1001
`define C_NP_P_O    4'b1010
`define C_NP_P_D    4'b1011

`define C_B_NP_O    4'b1100
`define C_B_NP_D    4'b1101
`define C_B_P_O     4'b1110
`define C_B_P_D     4'b1111
