
//MIT License
//
//Copyright (c) 2018 tvelliott
//
//Permission is hereby granted, free of charge, to any person obtaining a copy
//of this software and associated documentation files (the "Software"), to deal
//in the Software without restriction, including without limitation the rights
//to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//copies of the Software, and to permit persons to whom the Software is
//furnished to do so, subject to the following conditions:
//
//The above copyright notice and this permission notice shall be included in all
//copies or substantial portions of the Software.
//
//THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
//SOFTWARE.




//////////////////////////////////////////////////
//author t.elliott 2018
//DDR I/Q (tx side) AT86RF215
//

module ddr_out (
  input rx_clk,  //rx_clk from at86rf215 to derive tx_clk.  no phase requirements between the two, but freq must match
  output reg tx_clk_out,  //tx clk, iq serial signal to at86rf215 @ 64Mhz (128 Mbps DDR)
  output reg data_out,  //tx data, iq serial signal to at86rf215 @ 64Mhz (128 Mbps DDR)
  input rst_ddr,
  input [12:0] data_i, 
  input [12:0] data_q,
  output reg next_data,
  input do_tx,       //this signal can be embedded in bit 0 of data_i[13:0] if embedded control is enabled in IQIFC0[0]
  output data_start,
  input [3:0] tx_sample_rate
);

reg [31:0] data_reg;
reg [3:0] data_cnt;
reg [11:0] zero_cnt;

//assign tx_clk_out = rx_clk;
assign tx_clk_out = 1'b1; 

reg data_out_a;
reg data_out_b;

reg data1;
reg data2;
reg data_h;

reg [1:0] do_tx_e;
reg [3:0] zero_insert;

and U1(data1, data_out_a, tx_clk_out);
and U2(data2, data_out_b, !tx_clk_out);
or U3(data_out, data1, data2); 

always @(negedge tx_clk_out) begin

  if(zero_insert==4'b0000) 
    next_data <= data_cnt[3]; //load new data_iq regs 

end

always @(posedge tx_clk_out) begin

    if(zero_cnt[11]==1'b0) 
      zero_cnt <= zero_cnt+1;

    do_tx_e[1:0] <= {do_tx_e[0], do_tx};

    if(do_tx_e[1:0]==2'b01) 
    begin
      data_out_a <= 1'b0; 
      data_out_b <= 1'b0; 
      zero_cnt[11:0] <= 12'b0;
      data_cnt <= 4'h0;
      data_start <= 1'b0;
    end 
    else 
    begin

      if(zero_cnt[11]==1'b1) 
      begin
        data_cnt <= data_cnt+1;
        data_start <= 1'b1;

        if(data_cnt==4'h0) 
        begin
          if(zero_insert[3:0]==4'b0000) 
          begin
            if(!do_tx) 
              data_reg <= { 13'b0, do_tx, 2'b01, 13'b0, 1'b0, 2'b00 };
            else 
            begin
              data_reg <= { data_i[12:0], do_tx, 2'b01, data_q[12:0], 1'b0, 2'b00 };
              zero_insert <= tx_sample_rate;
            end
            data_out_a <= 1'b1; 
            data_out_b <= 1'b0; 
          end 
          else 
          begin
            zero_insert <= zero_insert-1;
            data_out_a <= 1'b0; 
            data_out_b <= 1'b0; 
          end
        end 
        else 
        begin 
          data_out_a <= data_reg[31]; 
          data_out_b <= data_reg[30]; 
          data_reg <= { data_reg[29:0], 2'b00 };
        end
      end

    end
end

endmodule
