
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




module ddr_in (
    input clk,
    input data_in,
    input rst,
    output reg [31:0] data_out_a, 
    output reg [31:0] data_out_b
);

wire data_in;
reg qb_d;
reg qb_q;

always @(*) begin
  qb_q = qb_d;

  if(!clk) begin
    qb_d = data_in;
  end 
end

always @(posedge clk) begin

    qb_q <= qb_d;

    if (rst) begin
      data_out_a <= 32'b0;
      data_out_b <= 32'b0;
    end else begin

      data_out_a <= { data_out_a[30:0],data_in };
      data_out_b <= { data_out_b[30:0],qb_q };
    end
end

endmodule
