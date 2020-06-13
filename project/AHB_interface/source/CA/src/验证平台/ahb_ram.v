//==============================================================================
//**********************************************************
//**********  Copyright (c) 2009 by Jiaxing Wu.  ***********
//**********       All Rights Reserved.          ***********
//**********************************************************
//
//  File Name    : *.v
//  Author       : Jiaxing Wu
//  Creation Date: */*/2009
//  Version      : 1.0
//
//  Modified by  : Jiaxing Wu
//  Date         : */*/2009
//  Revision     : 1.*
//***********************************************************
//  Discription: Can't support read to write or write to read
//               trasnfer without idle status
//
//==============================================================================

//`timescale 1ns/1ps

// Include files

`define RAM_AWIDTH 16

module ahb_ram (
   // ahb signals
    input  wire        hclk
   ,input  wire        hresetn
   ,input  wire        hsel
   ,input  wire [31:0] haddr
   ,input  wire [1:0]  htrans
   ,input  wire [2:0]  hsize
   ,input  wire [2:0]  hburst
   ,input  wire        hwrite
   ,input  wire [31:0] hwdata
   ,output reg         hready_resp
   ,output wire [1:0]  hresp
   ,output wire [31:0] hrdata
   // memory signals
   ,output wire [`RAM_AWIDTH-1:0] ram_addr
   ,output wire                   ram_we
   ,output wire [3:0]             ram_be
   ,output wire [31:0]            ram_din
   ,output wire                   ram_en
   ,input  wire [31:0]            ram_dout
   );

//--------------------------------------
//  Pin Declarations
//--------------------------------------

//--------------------------------------
//  Constant Declarations
//--------------------------------------

//--------------------------------------
//  Register and Wire Declarations
//--------------------------------------
   reg  [31:0] haddr_reg  ;
   reg  [3:0]  byte_en;
   reg  [3:0]  byte_en_reg;
   reg         wr_en_reg;
   wire        wr_en;
   wire        rd_en;
   wire        ready_en;
                          
//------------------------------------------------------------------------------
//  Beginning of Main Body
//  ======================
//------------------------------------------------------------------------------
// lock the ahb signals to registers
   always @(posedge hclk or negedge hresetn)
   begin
      if (!hresetn)
      begin
         haddr_reg  <= 32'h00000000;
      end
      else      
      begin
         haddr_reg  <= haddr  ;
      end
   end
      
// translate hsize to byte enable
   always @(hsize or haddr[1:0])
   begin
      case (hsize)
         3'b000: begin
            case (haddr[1:0])
               2'b00:   byte_en = 4'b0001;
               2'b01:   byte_en = 4'b0010;
               2'b10:   byte_en = 4'b0100;
               2'b11:   byte_en = 4'b1000;
               default: byte_en = 4'b0000;
            endcase
         end
         3'b001: begin
            case (haddr[1:0])
               2'b00,2'b01: byte_en = 4'b0011;
               2'b10,2'b11: byte_en = 4'b1100;
               default:     byte_en = 4'b0000;
            endcase
         end
         3'b010: begin
            byte_en = 4'b1111;
         end
         default: begin
            byte_en = 4'b0000;
         end
      endcase
   end
   
   always @(posedge hclk or negedge hresetn)
   begin
      if (!hresetn)
         byte_en_reg <= 4'b0000;
      else
      begin
         byte_en_reg <= byte_en;
      end
   end

   assign wr_en = hsel & htrans[1] & hwrite; 
   assign rd_en = hsel & htrans[1] & (~hwrite);
   assign ready_en = hsel & htrans[1];   
   
   always @(posedge hclk or negedge hresetn)
   begin
      if (!hresetn)
         wr_en_reg <= 1'b0;
      else
      begin
         if (wr_en)
            wr_en_reg <= 1'b1;
         else
            wr_en_reg <= 1'b0;
      end
   end

// Ram control signal : ram_addr
// wr_en_reg is changed to wr_en
   assign ram_addr = rd_en ? haddr[`RAM_AWIDTH-1:0] : (wr_en_reg ? 
                     haddr_reg[`RAM_AWIDTH-1:0] : {`RAM_AWIDTH{1'b0}});
              
// Ram control signal : ram_en
   assign ram_en = rd_en ? 1'b1 : (wr_en_reg ? 1'b1 : 1'b0);
   
// Ram control signal : ram_we
   assign ram_we = wr_en_reg;
   
// Ram control signal : be
   assign ram_be = rd_en ? byte_en : (wr_en_reg ? byte_en_reg : 4'b0000);
   
// Ram control signal : ram_din[31:0]
   assign ram_din = wr_en_reg ? hwdata : 4'b0000;

// Ram control signal : ram_rd
   //assign ram_rd = rd_en;
   
//----------------------------------------------------------
// Ahb response signals
//----------------------------------------------------------
// Ahb response signal : hready_resp  
   always @(posedge hclk or negedge hresetn)
   begin
      if (!hresetn)
         hready_resp <= #3 1'b0;
      else
      begin
         if (ready_en)
            hready_resp <= #3 1'b1;
         else
            hready_resp <= #3 1'b0;
      end
   end
        
// Ahb response signal : hresp[1:0]   
   assign #3 hresp = 2'b00;

// Ahb response signal : hrdata[31:0]
   assign #3 hrdata = ram_dout;
      
//------------------------------------------------------------------------------
//  Instantiation of Module
//------------------------------------------------------------------------------
  
//=======================  RTL End  ============================================
endmodule