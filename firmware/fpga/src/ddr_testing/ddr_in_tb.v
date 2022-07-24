
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



`timescale 10ns/10ns

`include "ddr_in.v"

module ddr_in_tb();

    reg clk;
    reg rst;
    reg data_in_t;
    wire signed[31:0] data_out_a_t;
    wire signed[31:0] data_out_b_t;

    reg sample_clk;

  integer               file_inputs;
  integer               file_outputs;
  integer status;
  integer clock_count;


   ddr_in ddr_in_1 (
      .clk(clk),
      .rst(rst),
      .data_in(data_in_t),
      .data_out_a(data_out_a_t),
      .data_out_b(data_out_b_t)
   );


   always
      #1   clk   =  ~clk;


   initial
   begin
      clk      =  1'b0;
      rst = 1;
      clock_count=0;

      $dumpfile("ddr_in.vcd");
      $dumpvars(0,ddr_in_1);

      file_inputs = $fopen("inputs.txt","r"); 
      file_outputs = $fopenw("outputs.txt"); 

      #4
      rst=0;

      #250000
      $finish();
   end

   //always @(posedge clk) begin
   //  clock_count = clock_count+1;
   //  if(clock_count==32 ) begin
   //   sample_clk=1;
   //   clock_count=0;
   //  end else if(clock_count==2) begin
   //   sample_clk=0;
   //  end
   //end

   always @(posedge clk ) begin
        $fwrite(file_outputs, "%d,%d\n", data_out_a_t, data_out_b_t);  
   end


   always @(clk) begin
      status = $fscanf(file_inputs,"%d,", data_in_t);
   end

   
endmodule
