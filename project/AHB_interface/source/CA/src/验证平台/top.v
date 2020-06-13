module top;
  reg clk;
  reg reset;
  
  // master1 signals
  wire hgrant1;
  wire hbusreq1;
  wire hwrite1;
  wire [1:0] htrans1;
  wire [2:0] hburst1;
  wire [2:0] hsize1;
  wire [31:0] hwdata1;
  wire [31:0] haddr1;
  
  // master2 signals
  wire hgrant2;
  wire hbusreq2;
  wire hwrite2;
  wire [1:0] htrans2;
  wire [2:0] hburst2;
  wire [2:0] hsize2;
  wire [31:0] hwdata2;
  wire [31:0] haddr2;
  
  // signals that are connected to all masters 
  wire hready;
  wire [1:0] hresp;
  wire [31:0] hrdata;
  
  // slave1 signals
  wire hready_s1;
  wire [1:0] hresp_s1;
  wire [31:0] hrdata_s1;
  wire hsel_s1;
  wire [15:0] ram_addr1;
  wire ram_we1;
  wire ram_en1;
  wire [3:0] ram_be1;
  wire [31:0] ram_din1;
  wire [31:0] ram_dout1;
  
  // slave2 signals
  wire hready_s2;
  wire [1:0] hresp_s2;
  wire [31:0] hrdata_s2;
  wire hsel_s2;
  wire [15:0] ram_addr2;
  wire ram_we2;
  wire ram_en2;
  wire [3:0] ram_be2;
  wire [31:0] ram_din2;
  wire [31:0] ram_dout2;
  
  // signals that are connected to all slaves
  wire [31:0] haddr_s;
  wire [2:0] hburst_s;
  wire [2:0] hsize_s;
  wire [1:0] htrans_s;
  wire [31:0] hwdata_s;
  wire hwrite_s;

  AHB_if_a master1(
                  .clk        (clk),
                  .reset      (reset),
                  .hready     (hready),
                  .hgrant     (hgrant1),
                  .hbusreq    (hbusreq1),
                  .hwrite     (hwrite1),
                  .htrans     (htrans1),
                  .hresp      (hresp),
                  .hburst     (hburst1),
                  .hsize      (hsize1),
                  .hrdata     (hrdata),
                  .hwdata     (hwdata1),
                  .haddr      (haddr1)
                  );
  
  AHB_if_b master2(
                  .clk        (clk),
                  .reset      (reset),
                  .hready     (hready),
                  .hgrant     (hgrant2),
                  .hbusreq    (hbusreq2),
                  .hwrite     (hwrite2),
                  .htrans     (htrans2),
                  .hresp      (hresp),
                  .hburst     (hburst2),
                  .hsize      (hsize2),
                  .hrdata     (hrdata),
                  .hwdata     (hwdata2),
                  .haddr      (haddr2)
                );
                  
  DW_ahb ahb_bus(
                  .hclk         (clk),
                  .hresetn      (reset),
                  
                  .haddr_m1     (haddr1),
                  .hbusreq_m1   (hbusreq1),
                  .hburst_m1    (hburst1),
                  .hsize_m1     (hsize1),
                  .htrans_m1    (htrans1),
                  .hwdata_m1    (hwdata1),
                  .hwrite_m1    (hwrite1),
                  .hgrant_m1    (hgrant1),
                  
                  .haddr_m2     (haddr2),
                  .hbusreq_m2   (hbusreq2),
                  .hburst_m2    (hburst2),
                  .hsize_m2     (hsize2),
                  .htrans_m2    (htrans2),
                  .hwdata_m2    (hwdata2),
                  .hwrite_m2    (hwrite2),
                  .hgrant_m2    (hgrant2),
                  
                  .hready       (hready),
                  .hresp        (hresp),
                  .hrdata       (hrdata),
                  
                  .hready_resp_s1 (hready_s1),
                  .hresp_s1       (hresp_s1),
                  .hrdata_s1      (hrdata_s1),
                  .hsel_s1        (hsel_s1),
                  
                  .hready_resp_s2 (hready_s2),
                  .hresp_s2       (hresp_s2),
                  .hrdata_s2      (hrdata_s2),
                  .hsel_s2        (hsel_s2),
                  
                  .haddr          (haddr_s),
                  .hburst         (hburst_s),
                  .hsize          (hsize_s),
                  .htrans         (htrans_s),
                  .hwdata         (hwdata_s),
                  .hwrite         (hwrite_s)
                );
                  
  ahb_ram slave1(
                    .hclk         (clk),
                    .hresetn       (reset),
                    .hwrite       (hwrite_s),
                    .hsel         (hsel_s1),
                    .htrans       (htrans_s),
                    .hsize        (hsize_s),
                    .hburst       (hburst_s),
                    .haddr        (haddr_s),
                    .hwdata       (hwdata_s),
                    .hready_resp  (hready_s1),
                    .hresp        (hresp_s1),
                    .hrdata       (hrdata_s1),
                    
                    .ram_addr     (ram_addr1),
                    .ram_we       (ram_we1),
                    .ram_be       (ram_be1),
                    .ram_din      (ram_din1),
                    .ram_en       (ram_en1),
                    .ram_dout     (ram_dout1)
                  );
       
  ahb_ram slave2(
                    .hclk         (clk),
                    .hresetn       (reset),
                    .hwrite       (hwrite_s),
                    .hsel         (hsel_s2),
                    .htrans       (htrans_s),
                    .hsize        (hsize_s),
                    .hburst       (hburst_s),
                    .haddr        (haddr_s),
                    .hwdata       (hwdata_s),
                    .hready_resp  (hready_s2),
                    .hresp        (hresp_s2),
                    .hrdata       (hrdata_s2),
                    
                    .ram_addr     (ram_addr2),
                    .ram_we       (ram_we2),
                    .ram_be       (ram_be2),
                    .ram_din      (ram_din2),
                    .ram_en       (ram_en2),
                    .ram_dout     (ram_dout2)
                  );
  ram my_ram1(
              .clk              (clk),
              .hresetn          (reset),
              .ram_addr         (ram_addr1),
              .ram_we           (ram_we1),
              .ram_en           (ram_en1),
              .ram_be           (ram_be1),
              .ram_din          (ram_din1),
              .ram_dout         (ram_dout1)
              );
   
  ram my_ram2(
              .clk              (clk),
              .hresetn          (reset),
              .ram_addr         (ram_addr2),
              .ram_we           (ram_we2),
              .ram_en           (ram_en2),
              .ram_be           (ram_be2),
              .ram_din          (ram_din2),
              .ram_dout         (ram_dout2)
              );
  
  initial
  begin
    //$monitor("%d    %d   %d    %d    %d   %d\n",my_ram1.ram_addr,my_ram1.ram_en,my_ram1.ram_we,my_ram1.ram_be,my_ram1.ram_din,my_ram1.ram_dout);
    //$monitor("%d    %d   %d   %d    %d    %d    %d    %d\n",slave1.hsize,slave1.haddr,slave1.hsel,slave1.ram_addr,slave1.hwdata,slave1.hrdata,ahb_bus.hrdata_s1,ahb_bus.hrdata);

  end                 
  initial
  begin
    clk=0;
    reset=1;
    #30 reset=0;
    #50 reset=1;
  end
  
  always
  begin
    #10 clk=~clk;
  end

endmodule   
