////////////////////////////////////////////////////////////////////////////////
//
//       This confidential and proprietary software may be used only
//     as authorized by a licensing agreement from Synopsys Inc.
//     In the event of publication, the following notice is applicable:
//
//                    (C) COPYRIGHT 2000 - 2005 SYNOPSYS INC.
//                           ALL RIGHTS RESERVED
//
//       The entire notice above must be reproduced on all authorized
//     copies.
//
// AUTHOR:    Reto Zimmermann         Jul 10, 2000
//
// VERSION:   Verilog Simulation Architecture
//
// DesignWare_version: b6fa7db8
// DesignWare_release: X-2005.09-DWF_0509
//
////////////////////////////////////////////////////////////////////////////////
//
// ABSTRACT:  Arbiter with two-level priority scheme
//            - first level: dynamic priority scheme
//            - second level: Fair-Among-Equal priority scheme
//   
// MODIFIED:
//          06/14/01  RPH    Fixed the X-processing. STAR 119685
//          12/21/01  RPH    Fixed the init priority bug due to order of 
//                           precedence of the operators and priority_low_int
//                           array not being in the sensitivity list.
//
//         03/26/03  RJK      Fixed problem with with int array triggering
//                            a combinational always block (STAR 160408)
//
//-----------------------------------------------------------------------------

module DW_arbiter_2t (clk, rst_n, request, priority, lock, mask, 
			   parked, granted, locked, grant, grant_index);

  parameter n          = 4;
  parameter p_width    = 2;
  parameter park_mode  = 1;
  parameter park_index = 0;
  parameter output_mode = 1;
   

  `define index_width ((n >16)?((n >64)?((n >128)?8:7):((n >32)?6:5)):((n >4)?((n >8)?4:3):((n >2)?2:1)))

  input  clk, rst_n;
  input  [n-1 : 0] request, lock, mask;
  input  [p_width*n-1 : 0] priority;
  output parked, granted, locked;
  output [n-1 : 0] grant;
  output [`index_width-1 : 0] grant_index;

  wire clk, rst_n;
  wire [n-1 : 0] request, lock, mask;
  wire [p_width*n-1 : 0] priority;
  wire  parked, locked;
  wire granted;
  wire [n-1 : 0] grant;
  wire [`index_width-1 : 0] grant_index;

  // synopsys translate_off

  integer priority_low_int[n-1 : 0], priority_low_next[n-1 : 0];  
  integer grant_index_int, grant_index_next;
  reg     parked_next, granted_int, granted_next, locked_next; 
  reg 	  parked_int, locked_int; 

  reg [n-1 : 0] request_masked_v;
  reg parked_v, granted_v, locked_v;
  reg request_x, lock_x, mask_x, priority_x;
  reg [p_width*n-1 : 0] priority_tmp;
  integer priority_high_v[n-1 : 0],  priority_low_v[n-1 : 0];
  integer grant_index_v;
  integer update_priorities_v, i;
  event priority_low_changed, update_arbitration;


  //---------------------------------------------------------------------------
  // Behavioral model
  //---------------------------------------------------------------------------
   always @(grant_index_int or granted_int or request or lock or mask or 
		    priority or update_arbitration) begin : PROC_arbitrate
		request_x  = ^request;
		lock_x     = ^lock;
		mask_x     = ^mask;
		priority_x = ^priority;
		
		grant_index_v = -1;
		parked_v      = 1'b0;
		granted_v     = 1'b0;
		locked_v      = 1'b0;
		
		for (i = 0; i < n; i = i+1) priority_low_v[i] = priority_low_int[i];
		update_priorities_v = 0;
		priority_tmp = priority;
		for (i = 0; i < n; i = i+1) begin
           priority_high_v[i] = priority_tmp[p_width-1 : 0];
           priority_tmp  = priority_tmp >> p_width;
		end
		request_masked_v = request & ~mask;

		if ((grant_index_int < -1) && (lock !== {n{1'b0}})) begin
			  grant_index_v = -2;
			  locked_v      = 1'bx;
			  granted_v     = 1'bx;
		end
		else if ((grant_index_int >= 0) && (lock[grant_index_int] !== 1'b0)) begin
		   if (lock[grant_index_int] === 1'b1) begin
			  grant_index_v = grant_index_int;
			  locked_v      = 1'b1;
			  granted_v     = 1'b1;
			  if (granted_int === 1'b0) update_priorities_v = 1;
		   end
		   else begin
			  grant_index_v = -2;
			  locked_v      = 1'bx;
			  granted_v     = 1'bx;
			  parked_v      = 1'b0;
		   end 
		end 
		else if (request_masked_v !== {n{1'b0}}) begin
		   if (request_x === 1'bx || priority_x === 1'bx ) begin
			  grant_index_v = -2;
			  granted_v = 1'bx;
			  parked_v      = 1'bx;
		   end 
		   else begin
			  for (i = 0; i < n; i = i+1) begin
				 if (request_masked_v[i] === 1'b1) begin
					if ((grant_index_v < 0) || 
						((priority_high_v[i] < priority_high_v[grant_index_v]) ||
						 ((priority_high_v[i] == priority_high_v[grant_index_v]) &&
						  (priority_low_int[i] < priority_low_int[grant_index_v])))) begin
					   grant_index_v = i;
					   granted_v = 1'b1;
					end
				 end
			  end
			  update_priorities_v = 1;
		   end 
		end 
		else if (park_mode == 1) begin	   
		   grant_index_v = park_index;
		   parked_v      = 1'b1;
		end
		else begin
           grant_index_v = -1;
		end

		for (i = 0; i < n; i = i+1) begin
           if (i == grant_index_v) begin
			  priority_low_v[i] = (1<<`index_width)-1;
           end
		   else if (request_masked_v[i] == 1'b1) begin
			  if (update_priorities_v == 1)begin
				if (priority_low_v[i] == 0)
				  priority_low_v[i] = (1<<`index_width)-1;
				else
				  priority_low_v[i] = priority_low_int[i] - 1;
			  end
           end
		   else begin
			  priority_low_v[i] = (1<<`index_width)-1;
           end
		end // for

		grant_index_next = grant_index_v;
		parked_next      = parked_v;
		granted_next     = granted_v;
		locked_next      = locked_v;
		for (i = 0; i < n; i = i+1) priority_low_next[i] = priority_low_v[i];
	 end // arbitrate

  always @(posedge clk or negedge rst_n) begin : PROC_register
    if (rst_n === 1'b0) begin
      grant_index_int <= -1;
      parked_int          <= 1'b0;
      granted_int     <= 1'b0;
      locked_int          <= 1'b0;
      for (i = 0; i < n; i = i+1) priority_low_int[i] <= (1<<`index_width)-1;
      -> priority_low_changed;
    end else if (rst_n === 1'b1) begin
      grant_index_int <= grant_index_next;
      parked_int          <= parked_next;
      granted_int     <= granted_next;
      locked_int          <= locked_next;
      for (i = 0; i < n; i = i+1) begin
	if (priority_low_int[i] !== priority_low_next[i]) begin
	  priority_low_int[i] <= priority_low_next[i];
	  -> priority_low_changed;
	end
      end // for
    end else begin
      grant_index_int <= -2;
      parked_int          <= 1'bx;
      granted_int     <= 1'bx;
      locked_int          <= 1'bx;
      for (i = 0; i < n; i = i+1) begin
        if (priority_low_int[i] !== -1) begin
          priority_low_int[i] <= -1;
          -> priority_low_changed;
        end
      end // for
    end
  end // register


  always @ (priority_low_changed) begin : PROC_trigger_update
    #1;
    -> update_arbitration;
  end

  assign grant = (grant_index_int == -2)? {n{1'bx}} :
		 (grant_index_next == -1 && output_mode == 0) ? {n{1'b0}} :
		 (output_mode == 0 ) ? 1'b1 << grant_index_next :
                 1'b1 << grant_index_int;
   assign grant_index = (grant_index_int == -2)? {`index_width{1'bx}} :
                       (grant_index_int == -1 && output_mode == 1) ? {`index_width{1'b1}} :
	               (grant_index_next == -1 && output_mode == 0) ? {`index_width{1'b1}} :
                       (output_mode == 0) ? grant_index_next[`index_width-1:0] :
                       grant_index_int[`index_width-1:0];
  assign granted = output_mode == 0 ? granted_next : 
	           granted_int;
   
  assign parked =  output_mode == 0 ? parked_next : 
	           parked_int;
  assign locked = output_mode == 0 ? locked_next : 
	          locked_int;
   
   


  //---------------------------------------------------------------------------
  // Parameter legality check and initializations
  //---------------------------------------------------------------------------
  
  initial begin : parameter_check
    integer param_err_flg;

    param_err_flg = 0;
    
    
    if ( (n < 2) || (n > 32) ) begin
      param_err_flg = 1;
      $display(
	"ERROR: %m :\n  Invalid value (%d) for parameter n (legal range: 2 to 32)",
	n );
    end
    
    if ( (p_width < 1) || (p_width > 5) ) begin
      param_err_flg = 1;
      $display(
	"ERROR: %m :\n  Invalid value (%d) for parameter p_width (legal range: 1 to 5)",
	p_width );
    end
    
    if ( (park_mode < 0) || (park_mode > 1) ) begin
      param_err_flg = 1;
      $display(
	"ERROR: %m :\n  Invalid value (%d) for parameter park_mode (legal range: 0 to 1)",
	park_mode );
    end
    
    if ( (park_index < 0) || (park_index > n-1) ) begin
      param_err_flg = 1;
      $display(
	"ERROR: %m :\n  Invalid value (%d) for parameter park_index (legal range: 0 to n-1)",
	park_index );
    end
    
    if ( (output_mode < 0) || (output_mode > 1) ) begin
      param_err_flg = 1;
      $display(
	"ERROR: %m :\n  Invalid value (%d) for parameter output_mode (legal range: 0 to 1)",
	output_mode );
    end
  
    if ( param_err_flg == 1) begin
      $display(
        "%m :\n  Simulation aborted due to invalid parameter value(s)");
      $finish;
    end

  end // parameter_check

  //---------------------------------------------------------------------------
  // Report unknown clock inputs
  //---------------------------------------------------------------------------
  
  always @ (clk) begin : clk_monitor 
    if ( (clk !== 1'b0) && (clk !== 1'b1) && ($time > 0) )
      $display( "WARNING: %m :\n  at time = %t, detected unknown value, %b, on clk input.",
                $time, clk );
    end // clk_monitor 
    
  // synopsys translate_on

endmodule

//-----------------------------------------------------------------------------
