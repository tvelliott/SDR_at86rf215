
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



module bram_256_16(input clk, ren, wen, input [9:0] raddr, input [9:0] waddr, input [15:0] wdata, output reg [15:0] rdata);

  reg [15:0] mem [0:1023];

  always @(posedge clk) begin
    if (wen) mem[waddr] <= wdata;
    if(ren) rdata <= mem[raddr];
  end

endmodule

module bram_256_16_dual(input rclk, input wclk, ren, wen, input [7:0] raddr, input [7:0] waddr, input [15:0] wdata, output reg [15:0] rdata);

  reg [15:0] mem [0:255];

  always @(posedge rclk) begin
    //if(ren) rdata <= mem[raddr];
    rdata <= mem[raddr];
  end

  always @(posedge wclk) begin
    if(wen) mem[waddr] <= wdata;
  end

endmodule
