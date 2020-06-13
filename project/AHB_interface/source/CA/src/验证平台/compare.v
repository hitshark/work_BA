module comparator(a_gt_b,a_lt_b,a_eq_b,a,b);
  output a_gt_b;
  output a_lt_b;
  output a_eq_b;
  
  input [3:0] a,b;
  
  assign a_gt_b = (a > b);
  assign a_lt_b = (a < b);
  assign a_eq_b = (a == b);
endmodule