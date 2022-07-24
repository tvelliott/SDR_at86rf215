
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
//DDR I/Q data receiver for use with AT86RF215
//
// rising edge of data_valid should be used to latch data_i and data_q
//

module ddr_in (
    input clk,
    input data_in,
    input rst_ddr,
    output wire [12:0] data_i, 
    output wire [12:0] data_q,
    output reg sync,
    output reg data_valid
);

reg qa_q;
reg qb_q;
reg sync;
reg dv;

reg [31:0] data_reg;
reg [3:0] sync_cnt;
reg [3:0] sync_to;

always @(negedge clk) begin
    qb_q <= data_in;  //Q data on falling edge
end

always @(posedge clk) begin

    if (rst_ddr) begin
      data_reg <= 32'b0;
      data_i <= 13'b0;
      data_q <= 13'b0;
      sync <= 1'b0;
      dv <= 1'b0;
      data_valid <= 1'b0;
    end 
    else 
    begin

      qa_q <= data_in;  //I data on rising edge

      data_reg <= { data_reg[29:0], qa_q, qb_q }; //shift in 2 bits every rising edge

      sync_cnt <= sync_cnt+1;

      if(!sync) 
      begin
        //initial sync check to reset sync_cnt clock
        if(data_reg[31:30]==2'b10 && data_reg[16]==1'b0 && data_reg[15:14]==2'b01 && data_reg[0]==1'b0) //sync bit pattern for AT86RF215
        begin  
          sync_cnt <= 4'h1;
          sync <= 1'b1;
          sync_to <= 4'hb;
        end
      end 
      else 
      begin 

        if(sync_cnt==4'h0) 
        begin
          if(data_reg[31:30]==2'b10 && data_reg[16]==1'b0 && data_reg[15:14]==2'b01 && data_reg[0]==1'b0) //sync bit pattern for AT86RF215
          begin  
            sync_to <= 4'hb;  //we are still synced, so reset the timeout counter
            dv<=1'b1;
          end 
          else 
          begin
            if(sync_to!=4'h0) 
              sync_to <= sync_to-1;
            else 
              sync <= 1'b0; //lost sync
          end
        end

      end

      if(sync_cnt[3]==1'b1) 
          dv<=1'b0;

      //generate a data_valid signal with frequency of Fs (400Khz - 4Mhz). data_valid rising edge is centered on the I/Q data words
      if(dv && sync_cnt[3]==1'b0) 
      begin
        data_i[12:0] <= data_reg[29:17];
        data_q[12:0] <= data_reg[13:1];
        data_valid <= 1'b1;
      end 
      else 
        data_valid <= 1'b0;
    end
end

endmodule
