module ram #(
 parameter data_width = 32, // works only for 32 bit
 parameter addr_width = 16,
 parameter bena_width = data_width / 8 
 )
(
 input clk,
 input hresetn,
 input [addr_width-1:0] ram_addr,
 input ram_we,
 input ram_en,
 input [data_width-1:0] ram_din,
 input [bena_width-1:0] ram_be,
 output reg [data_width-1:0] ram_dout
 );
 
 reg    [7:0]      memory[0:4096];
 always @(posedge clk or negedge hresetn)
 begin
   if(!hresetn)begin
     ram_dout <= 0;
      end
   else begin
     if(ram_en)begin
     if(ram_we)begin
      case(ram_be)
        4'b0001,4'b0010,4'b0100,4'b1000:memory[ram_addr] <= ram_din[7:0];
        //4'b0010:memory[ram_addr+1]=ram_din[7:0];
        //4'b0100:memory[ram_addr+2]=ram_din[7:0];
        //4'b1000:memory[ram_addr+3]=ram_din[7:0];
        4'b0011,4'b1100:begin
          memory[ram_addr] <= ram_din[7:0];
          memory[ram_addr+1] <= ram_din[15:8];
        end
        /*4'b1100:begin
          memory[ram_addr+2]=ram_din[7:0];
          memory[ram_addr+3]=ram_din[15:8];
        end */
        4'b1111:begin
          memory[ram_addr] <= ram_din[7:0];
          memory[ram_addr+1] <= ram_din[15:8];
          memory[ram_addr+2] <= ram_din[23:16];
          memory[ram_addr+3] <= ram_din[31:24];
        end
        default:;
      endcase
    end
  else
    begin
      case(ram_be)
        4'b0001,4'b0010,4'b0100,4'b1000:ram_dout[7:0] <= memory[ram_addr];
        //4'b0010:ram_dout[7:0]=memory[ram_addr+1];
        //4'b0100:ram_dout[7:0]=memory[ram_addr+2];
        //4'b1000:ram_dout[7:0]=memory[ram_addr+3];
        4'b0011,4'b1100:begin
          ram_dout[7:0] <= memory[ram_addr];
          ram_dout[15:8] <= memory[ram_addr+1];
        end
       /* 4'b1100:begin
          ram_dout[7:0]=memory[ram_addr+2];
          ram_dout[15:8]=memory[ram_addr+3];
        end */
        4'b1111:begin
          ram_dout[7:0] <= memory[ram_addr];
          ram_dout[15:8] <= memory[ram_addr+1];
          ram_dout[23:16] <= memory[ram_addr+2];
          ram_dout[31:24] <= memory[ram_addr+3];
        end
        default:;
      endcase
    end
  end
  end     
end

endmodule
