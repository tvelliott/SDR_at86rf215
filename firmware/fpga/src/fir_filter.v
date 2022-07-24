
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



//verilog filter generator
//@author t.elliott 2017
//filter auto-generated:  FilterBuilder/filterbuilder 16 0.01 0 hann 0 18.0

module fir_filter (
  input clk,
  input clk_en,
  input rst,
  input signed [7:0] filter_in,
  output signed [16:0] filter_out17
);

reg signed [16:0] outputreg;

  reg signed [7:0] coef1;
  reg signed [7:0] coef2;
  reg signed [7:0] coef3;
  reg signed [7:0] coef4;
  reg signed [7:0] coef5;
  reg signed [7:0] coef6;
  reg signed [7:0] coef7;
  reg signed [7:0] coef8;
  reg signed [7:0] coef9;
  reg signed [7:0] coef10;
  reg signed [7:0] coef11;
  reg signed [7:0] coef12;
  reg signed [7:0] coef13;
  reg signed [7:0] coef14;
  reg signed [7:0] coef15;
  reg signed [7:0] coef16;
  reg signed [7:0] coef17;

  reg signed [7:0] fifo_0;
  reg signed [7:0] fifo_1;
  reg signed [7:0] fifo_2;
  reg signed [7:0] fifo_3;
  reg signed [7:0] fifo_4;
  reg signed [7:0] fifo_5;
  reg signed [7:0] fifo_6;
  reg signed [7:0] fifo_7;
  reg signed [7:0] fifo_8;
  reg signed [7:0] fifo_9;
  reg signed [7:0] fifo_10;
  reg signed [7:0] fifo_11;
  reg signed [7:0] fifo_12;
  reg signed [7:0] fifo_13;
  reg signed [7:0] fifo_14;
  reg signed [7:0] fifo_15;
  reg signed [7:0] fifo_16;

  reg signed [24:0] sum;

  always @( posedge clk)
    begin
      if (!rst) begin

        fifo_0 <= 0;
        fifo_1 <= 0;
        fifo_2 <= 0;
        fifo_3 <= 0;
        fifo_4 <= 0;
        fifo_5 <= 0;
        fifo_6 <= 0;
        fifo_7 <= 0;
        fifo_8 <= 0;
        fifo_9 <= 0;
        fifo_10 <= 0;
        fifo_11 <= 0;
        fifo_12 <= 0;
        fifo_13 <= 0;
        fifo_14 <= 0;
        fifo_15 <= 0;
        fifo_16 <= 0;

        coef1 <= 0;
        coef2 <= 0;
        coef3 <= 2;
        coef4 <= 5;
        coef5 <= 8;
        coef6 <= 12;
        coef7 <= 15;
        coef8 <= 17;
        coef9 <= 18;
        coef10 <= 17;
        coef11 <= 15;
        coef12 <= 12;
        coef13 <= 9;
        coef14 <= 5;
        coef15 <= 2;
        coef16 <= 0;
        coef17 <= 0;

         sum <= 0;
      end
      else begin
        //if (clk_en== 1'b1) begin
          //
          sum <= (fifo_0 * coef1) +
               (fifo_1 * coef2) +
               (fifo_2 * coef3) +
               (fifo_3 * coef4) +
               (fifo_4 * coef5) +
               (fifo_5 * coef6) +
               (fifo_6 * coef7) +
               (fifo_7 * coef8) +
               (fifo_8 * coef9) +
               (fifo_9 * coef10) +
               (fifo_10 * coef11) +
               (fifo_11 * coef12) +
               (fifo_12 * coef13) +
               (fifo_13 * coef14) +
               (fifo_14 * coef15) +
               (fifo_15 * coef16) +
               (fifo_16 * coef17);

          outputreg <= sum;

            fifo_0 <= fifo_1;
            fifo_1 <= fifo_2;
            fifo_2 <= fifo_3;
            fifo_3 <= fifo_4;
            fifo_4 <= fifo_5;
            fifo_5 <= fifo_6;
            fifo_6 <= fifo_7;
            fifo_7 <= fifo_8;
            fifo_8 <= fifo_9;
            fifo_9 <= fifo_10;
            fifo_10 <= fifo_11;
            fifo_11 <= fifo_12;
            fifo_12 <= fifo_13;
            fifo_13 <= fifo_14;
            fifo_14 <= fifo_15;
            fifo_15 <= fifo_16;
          fifo_16 <= filter_in;

        //end
      end
    end


    assign filter_out17 = outputreg;

endmodule
