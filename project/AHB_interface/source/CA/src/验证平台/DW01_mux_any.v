// $Header: /remote/src/dware/jerb/dev/dw/dw01/src_ver/RCS/DW01_mux_any.v,v 1.3 1996/01/29 12:39:25 angelina Exp $
//-----------------------------------------------------------------------------------
//
// ABSTRACT:  Universal Multiplexer
//
//-------------------------------------------------------------------------------
//
//      WSFDB revision control info
//		@(#)DW01_mux_any.v	1.4
// MODIFIED: 
//
//           RPH        07/17/2002 
//                      Rewrote to comply with the new guidelines   
//-------------------------------------------------------------------------------
module DW01_mux_any(A,SEL,MUX);
   
   parameter	A_width = 8;
   parameter 	SEL_width = 2;
   parameter 	MUX_width = 2;
   
   input [A_width-1:0] A;
   input [SEL_width-1:0] SEL;

   output [MUX_width-1:0] MUX;

   // synopsys translate_off
     
   //-------------------------------------------------------------------------
   // Parameter legality check
   //-------------------------------------------------------------------------
    
  initial begin : parameter_check
    integer param_err_flg;

    param_err_flg = 0;
    
    
    if (A_width < 1) begin
      param_err_flg = 1;
      $display(
	"ERROR: %m :\n  Invalid value (%d) for parameter A_width (lower bound: 1)",
	A_width );
    end
    
    if (SEL_width < 1) begin
      param_err_flg = 1;
      $display(
	"ERROR: %m :\n  Invalid value (%d) for parameter SEL_width (lower bound: 1)",
	SEL_width );
    end
    
    if (MUX_width < 1) begin
      param_err_flg = 1;
      $display(
	"ERROR: %m :\n  Invalid value (%d) for parameter MUX_width (lower bound: 1)",
	MUX_width );
    end
    
    if ( param_err_flg == 1) begin
      $display(
        "%m :\n  Simulation aborted due to invalid parameter value(s)");
      $finish;
    end

  end // parameter_check
       
       function [MUX_width-1:0] DWF_mux;
	  input [A_width-1:0] a;
	  input [SEL_width-1:0] sel;

	  integer 		i,j;
	  reg [MUX_width-1:0] 	mux;
	  begin
	     for(i = 0;i < MUX_width;i = i+1) begin
		j = sel*MUX_width + i;
		if(j > A_width-1)
		   mux[i] = 1'b0;
		else
		   mux[i] = a[j];
	     end
	     DWF_mux = mux;
	  end
       endfunction

   assign  MUX[MUX_width-1:0] =  ((^(SEL ^ SEL) !== 1'b0)) ? {MUX_width{1'bx}} :
                              DWF_mux(A,SEL);
 
 // synopsys translate_on

endmodule

