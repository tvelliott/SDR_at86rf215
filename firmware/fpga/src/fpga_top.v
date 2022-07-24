
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



//author t.elliott 

//`include "pll.v"  //FSMC_CLK can be continuous on H7 part, so just use that @ 100 MHz
`include "bram_256_16.v"
`include "ddr_in.v"
`include "ddr_out.v"
`include "cic.v"


module top (
  input  clk_in,
  input  FPGA_RXCLK,
  output AT_RESET_N,
  output AT_SCLK,
  output AT_MOSI,
  input AT_MISO,
  input AT_IRQ,
  output AT_CS,
  input FPGA_RX900,
  input FPGA_RX24,
  output FPGA_TXCLK,
  output FPGA_TX_24_900,
  output LED2,
  output LED3,
  output AUDIO_SDIN,
  output AUDIO_SCLK,
  output AUDIO_LRCLK,
  output AUDIO_MCLK,
  output BACKLIGHT_EN,
  output TUNER_SCK,
  output TUNER_MOSI,
  output TUNER_CS,
  input TUNER_MISO,
  input BT1,
  input BT2,
  input BT3,
  input BT4,
  input BT5,
  input BT6,
  input CBSEL,
  output A0,  //audio interrupt
  output A1, //rf900 IQ rx fifo ready int
  output A2,   //button1
  output A3,   //button2
  output A4,  //button3
  output A5,  //button4
  output A6,  //button5
  output A7,  //button6
  input A8,  
  input A9, 
  input A10,  
  input A11, 
  input A12, 
  input A13, 
  input A14, 
  input A15,
  inout D0,
  inout D1,
  inout D2,
  inout D3,
  inout D4,
  inout D5,
  inout D6,
  inout D7,
  inout D8,
  inout D9,
  inout D10,
  inout D11,
  inout D12,
  inout D13,
  inout D14,
  inout D15,
  input FSMC_NOE,
  input FSMC_NWE,
  input FSMC_NE1,
  input FSMC_NE4,
  input FSMC_CLK_IN,
  input FSMC_NL_NADV,
  input FPGA_SPI_SCLK,
  input FPGA_SPI_MOSI,
  inout FPGA_SPI_MISO,  
  input FPGA_SPI_CS,
  output MIC_CLK,
  input MIC_DATA
);


  //TODO:  FIX THESE ADRESS LINES !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
  //assign do_tx_eec = A3;
  //assign A4 = tx_mem_addr[8]; //interrupt to mcu for tx fifo buffer change
  //TODO:  FIX THESE ADRESS LINES !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
 
  reg [10:0] mem_addr;

  reg led2;

  assign LED2 = led2;

  reg bl_en;
  assign BACKLIGHT_EN = bl_en;

  assign FSMC_NOE = noe;
  assign FSMC_NWE = nwe;
  assign FSMC_NE1 = ne1;
  assign FSMC_NL_NADV = nadv;

  reg [1:0] nadv_edge;
  reg [1:0] nwe_edge;
  reg [1:0] noe_edge;
  reg [1:0] dbus_out_en;

  //wire [15:0] abus;
  //assign abus = { A15,A14,A13,A12,A11,A10,A9,A8,A7,A6,A5,A4,A3,A2,A1,A0 };
 
  reg [7:0] tx_i_offset;
  reg [7:0] tx_q_offset;


  wire rx_clk_i;
  assign rx_clk_i = FPGA_RXCLK;
  reg rx_clk;

  SB_GB global_clock_buffer6 (
    .USER_SIGNAL_TO_GLOBAL_BUFFER (rx_clk_i),
    .GLOBAL_BUFFER_OUTPUT (rx_clk)
    );

  reg do_status_led;
  reg do_rand_lsb;



  reg [7:0] nwe_count;

  reg mic_clk;
  assign mic_clk = MIC_CLK;

  reg mic_data;
  reg prev_mic_data;
  assign MIC_DATA = mic_data;


    ddr_in ddr_in_rf900 (
      .clk(rx_clk),
      .data_in(rx_data_900),
      .rst_ddr(!rst_t),
      .data_i(rf900_i_data), 
      .data_q(rf900_q_data),
      .sync(rf900_iq_sync),
      .data_valid(rf900_ddr_datavalid),
    );



  reg tx_data_start;

  ddr_out ddr_out_rfxx (
    .tx_clk_out(tx_clk_n),
    .rx_clk(rx_clk),
    .data_out(tx_data_n),
    .rst_ddr(!rst_t),
    .data_i(tx_data_i),
    .data_q(tx_data_q),
    .next_data(tx_next_data),
    .do_tx( do_tx_eec ),
    .data_start(tx_data_start),
    .tx_sample_rate(tx_sample_rate_t)
  );

  bram_256_16_dual iq900_fifo_i1 (
    .rdata(I_iq900_fifo_data_r1),
    .raddr(mem_addr[7:0]),
    .rclk(clk),
    .ren(iq900_addr[8]==1'b0),
    .waddr(iq900_addr[7:0]),
    .wclk(cic_i_dvalid),  //rising edge
    .wdata( rf900_data_i16 ),
    .wen(iq900_addr[8]==1'b1)
  );

  bram_256_16_dual iq900_fifo_q1 (
    .rdata(Q_iq900_fifo_data_r1),
    .raddr(mem_addr[7:0]),
    .rclk(clk),
    .ren(iq900_addr[8]==1'b0),
    .waddr(iq900_addr[7:0]),
    .wclk(cic_q_dvalid),  //rising edge
    .wdata( rf900_data_q16 ),
    .wen(iq900_addr[8]==1'b1)
  );

  bram_256_16_dual iq900_fifo_i2 (
    .rdata(I_iq900_fifo_data_r2),
    .raddr(mem_addr[7:0]),
    .rclk(clk),
    .ren(iq900_addr[8]==1'b1),
    .waddr(iq900_addr[7:0]),
    .wclk(cic_i_dvalid),  //rising edge
    .wdata( rf900_data_i16 ),
    .wen(iq900_addr[8]==1'b0)
  );

  bram_256_16_dual iq900_fifo_q2 (
    .rdata(Q_iq900_fifo_data_r2),
    .raddr(mem_addr[7:0]),
    .rclk(clk),
    .ren(iq900_addr[8]==1'b1),
    .waddr(iq900_addr[7:0]),
    .wclk(cic_q_dvalid),  //rising edge
    .wdata( rf900_data_q16 ),
    .wen(iq900_addr[8]==1'b0)
  );

  bram_256_16_dual tx_iq900_fifo_iq1 (
    .rdata(IQ_iq900_fifo_data_t1[15:0]),
    .raddr(tx_mem_addr[7:0]),
    .rclk(tx_next_data),
    .ren(!tx_mem_addr[8]),
    .waddr(mem_addr[7:0]),
    .wclk(clk),  //rising edge
    .wdata(tx_rf900_data_iq16_f1[15:0]),
    .wen((!nwe && nadv && addr_in[15:11]==5'b00000))
  );

  bram_256_16_dual tx_iq900_fifo_iq2 (
    .rdata(IQ_iq900_fifo_data_t2[15:0]),
    .raddr(tx_mem_addr[7:0]),
    .rclk(tx_next_data),
    .ren(tx_mem_addr[8]),
    .waddr(mem_addr[7:0]),
    .wclk(clk),  //rising edge
    .wdata(tx_rf900_data_iq16_f2[15:0]),
    .wen((!nwe && nadv && addr_in[15:11]==5'b00100))
  );

  //MCU / FPGA 16-bit memory interface signals
  wire [15:0] dbus_in;
  reg [15:0] dbus_out;
  reg [15:0] data_in;
  reg [15:0] addr_in;
  wire noe;
  wire nwe;
  wire nadv;
  wire ne1; //select signal for external memory address range starting at 0x60000000 on stm32f4


  //lattice ICE40 specific high-z data lines so arachne-pnr can deal with it
  SB_IO #(
    .PIN_TYPE(6'b 1010_01),
    .PULLUP(1'b 0)
  ) dbus [15:0] (
    .PACKAGE_PIN({D15,D14,D13,D12,D11,D10,D9,D8,D7,D6,D5,D4,D3,D2,D1,D0}),
    .OUTPUT_ENABLE(dbus_out_en[1]),
    .D_OUT_0(dbus_out),
    .D_IN_0(dbus_in)
  );


  reg mco2;
  //assign mco2 = clk_in;

  reg mem_clk;
  //and U1(mem_clk, mco2, rx_clk); //AND gate

  reg clk;

  SB_GB global_clock_buffer5 (
    .USER_SIGNAL_TO_GLOBAL_BUFFER (FSMC_CLK_IN),
    .GLOBAL_BUFFER_OUTPUT (clk)
    );

//  SB_GB global_clock_buffer2 (
 //   .USER_SIGNAL_TO_GLOBAL_BUFFER (clk_in),
  //  .GLOBAL_BUFFER_OUTPUT (mem_clk)
   // );


  reg sync_led3;
  assign LED3 = !(sync_led3 && outcnt[0]);

  and U2(sync_led3,  iq900_addr[8], rf900_iq_sync); 
  assign A1 = iq900_addr[8];  //interrupt to mcu for fifo buffer change

    

  reg [15:0] temp_data;
  reg [15:0] test_data;


  //linear feedback shift register LFSR for noise waveform generation
  reg [15:0] rand16_1; 
  reg r16_lf_1; 

  //hw reset signal comes from MCU
  reg rst_t;
  assign rst_t = CBSEL; 


  //40MHz in,  83.333 MHz out
  //pll pll1 (
  //  .clock_in(clk),
  //  .clock_out(clk_t)
  //);

  //led counter and delays
  localparam BITS = 3;
  localparam LOG2DELAY = 23;
  reg [BITS+LOG2DELAY-1:0] counter;
  reg [BITS-1:0] outcnt;


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

assign AT_RESET_N = at86rf_reset_n;
assign AT_SCLK = at_sclk;
assign AT_MOSI = at_mosi; 
assign AT_IRQ = at86rf_irq; 
assign AT_CS = at_spi_cs;
assign AT_MISO = at_miso;

//assign FSMC_NE4 = mcu_int_exti12;  //exti_15_10

assign TUNER_SCK = tuner_sck;
assign TUNER_MOSI = tuner_mosi;
assign TUNER_CS = tuner_cs;
assign TUNER_MISO = tuner_miso;

assign FPGA_SPI_SCLK = fpga_mcu_sclk;
assign FPGA_SPI_MOSI = fpga_mcu_mosi;
assign FPGA_SPI_CS = fgpa_mcu_cs;

wire at_spi_cs;
wire at_miso;
wire at_mosi;
wire at_sclk;
reg at86rf_reset_n;
wire at86rf_irq;

wire fgpa_mcu_cs;
wire fpga_mcu_mosi;
wire fpga_mcu_sclk;
wire mcu_int_exti12;

reg miso_hi_z;
reg miso_out;


wire tuner_sck;
wire tuner_miso;
wire tuner_mosi;
wire tuner_cs;
reg tuner_out;
reg tuner_hi_z;

  //lattice ICE40 specific high-z data lines so arachne-pnr can deal with it
  SB_IO #(
    .PIN_TYPE(6'b 1010_01),
    .PULLUP(1'b 0)
  ) mcu_miso1 (
    .PACKAGE_PIN(FPGA_SPI_MISO),
    .OUTPUT_ENABLE(!fgpa_mcu_cs),  //FIX
    .D_OUT_0(miso_out),
    .D_IN_0(miso_hi_z)  //high-impedance, don't care
  );

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
always @(*) begin

     at_mosi = fpga_mcu_mosi;
     at_sclk = fpga_mcu_sclk;
     mcu_int_exti12 = at86rf_irq;

     tuner_cs = A10;
     at_spi_cs = A11; 

     tuner_mosi = fpga_mcu_mosi;
     tuner_sck = fpga_mcu_sclk;
     
  case( {A11,A10} ) 
     2'b01: begin 
       miso_out = at_miso;
     end

     2'b10: begin 
       miso_out = tuner_miso;
     end

    default: begin
      miso_out = 1'b0; 
    end
  endcase

      A2 = BT1;  
      A3 = BT2;  
      A4 = BT3;  
      A5 = BT4;  
      A6 = BT5;  
      A7 = BT6;  

end

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
always @(posedge clk) begin

   if(!rst_t) 
   begin
    outcnt <= 0; 
    //led2 <= 1'b1;
    at86rf_reset_n <= 1'b0;
    do_tx_eec <= 1'b0; //FIX
    do_rand_lsb <= 1'b0;
    bl_en <= 1'b0;
   end 
   else 
   begin


    bl_en <= 1'b1; 
    at86rf_reset_n <= 1'b1;

    //LED and TEST signals using counter
    counter <= counter + 1;

    //pwm brightness control of leds
    if( counter%2==0 ) 
     outcnt <= counter >> LOG2DELAY;
    else 
     outcnt <= 0; 

    //led2 <= ~outcnt[2];

    //linear feedback shift registers for some psuedo-randomness 
    if(counter[10]==0) 
    begin
      r16_lf_1 <= !(rand16_1[15]^rand16_1[3]);
      rand16_1 <= { rand16_1[14:0], r16_lf_1 }; 
    end

 end
end


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// latch memory bus address from MCU for the read/write operation to follow 
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
always @(posedge clk) begin

    if(!ne1) nwe_count <= nwe_count+1;

    dbus_out_en[1:0] <= { dbus_out_en[0], 1'b0 };

    nadv_edge[1:0] <= { nadv_edge[0], nadv };
    if(nadv_edge[1:0]==2'b01) //rising edge of NADV
    begin   
      addr_in <= dbus_in<<1;   
      nwe_count[7:0] <= 8'h00; 
      mem_addr <= dbus_in[10:0];
    end

    if(!noe) 
      dbus_out_en[1:0] <= 2'b11;

end

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// bus read to MCU <- from <- FPGA
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
always @(posedge clk) begin

    noe_edge[1:0] <= { noe_edge[0], noe };//falling edge of NOE

    if(noe_edge[1:0]==2'b01) 
    begin  

        case(addr_in[15:11]) 

              //0x1000
          5'b00010: begin
              dbus_out <= 16'h5555; 
          end
              //0x2000
          5'b00100: begin
              dbus_out <= 16'hAAAA; 
          end

          //address range 0xc0xx
          5'b11000: begin
            dbus_out <= rdata;
            //dbus_out <= audio_rdata;
          end

          //address range 0xf0xx
          5'b11110: begin
            dbus_out <= rand16_1;  //read-only 16-bit random number 
          end

          //address range 0x40xx
          5'b01000: begin
            dbus_out <= { I_iq900_fifo_data_r1[15:8], Q_iq900_fifo_data_r1[15:8] };  
          end

          //address range 0x50xx
          5'b01010: begin
            //dbus_out <= Q_iq900_fifo_data_r1[15:0];  
          end

          //address range 0x60xx
          5'b01100: begin
            dbus_out <= { I_iq900_fifo_data_r2[15:8], Q_iq900_fifo_data_r2[15:8] };  
          end

          //address range 0x70xx
          5'b01110: begin
            dbus_out <= test_data[15:0];  
            //test_data[15:0] <= 16'h0000; 
          end


          //address range 0x80xx
          //5'b10000: begin
           // dbus_out <= I_iq24_fifo_data_r1[15:0];  
          //end

          //address range 0x90xx
          //5'b10010: begin
           // dbus_out <= Q_iq24_fifo_data_r1[15:0];  
          //end

          //address range 0xa0xx
          //5'b10100: begin
           // dbus_out <= I_iq24_fifo_data_r2[15:0];  
          //end

          //address range 0xb0xx
          //5'b10110: begin
           // dbus_out <= Q_iq24_fifo_data_r2[15:0];  
          //end

          default begin
            dbus_out <= 16'h0000; 
          end


        endcase
    end 


end

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// bus write from MCU -> TO -> FPGA
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
always @(posedge clk) begin

     bram_lat[1:0] <= { bram_lat[0], 1'b0 };

     if(!nwe && nwe_count[7:0]==8'h04) 
     begin

        case(addr_in[15:11]) 

          //adress range 0xc0xx
          5'b11000: begin
            wdata <= dbus_in[15:0];
            //wdata <= rand16_1; 
            bram_lat <= 2'b11;
            //audio_wdata <= dbus_in[15:0];
            //audio_we[1:0] <= 2'b10; //1 cycle
            //audio_wdata <= rand16_1; 
            //audio_waddr[10:0] <= mem_addr[10:0];
          end

          //address range 0xd0xx
          5'b11010: begin
            bram_waddr[9:0] <= dbus_in[9:0];
            //audio_waddr[10:0] <= dbus_in[10:0];
          end

          //address range 0x70xx
          5'b01110: begin
            //test_data[15:0] <= dbus_in[15:0];
            //test_data[15:0] <= 16'h1234; 
            //test_data[15:0] <= rand16_1; 
            test_data[15:0] <= { 8'h00, nwe_count[7:0] }; 
          end

          //address range 0x00xx
          5'b00000: begin
            tx_rf900_data_iq16_f1 <= dbus_in[15:0]; 
          end
          //address range 0x10xx
          5'b00010: begin
            //tx_rf900_data_q16_f1 <= dbus_in[15:0];
          end
          //address range 0x20xx
          5'b00100: begin
            tx_rf900_data_iq16_f2 <= dbus_in[15:0]; //fifo 2
          end
          //address range 0x30xx
          5'b00110: begin
            //tx_rf900_data_q16_f2 <= dbus_in[15:0];
          end

          //address range 0x40xx  //write-only config
          5'b01000: begin
            case(addr_in[8:1])
              8'h00:  begin
                tx_sample_rate_t[3:0] = dbus_in[3:0]; 
              end
              //8'h01:  begin
               // do_rand_lsb <= dbus_in[0]; 
              //end
              8'h02:  begin
                do_status_led <= dbus_in[0]; 
              end
              8'h03:  begin
                tx_i_offset[7:0] <= dbus_in[7:0]; 
              end
              8'h04:  begin
                tx_q_offset[7:0] <= dbus_in[7:0]; 
              end
            endcase
          end



          default begin
            temp_data <= dbus_in; 
          end

        endcase
    end

end


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// output stereo audio to I2S dac via the outputs mclk, sclk, lrclk, and sdata
//
// with clk running @ 100 MHz, this configuration ends up with audio sample rate of 16.276 Ksps 
// mclk = 100e6/12 = 8.333 MHz
// sclk = 100e6/192 = 0.5208 MHz 
// Fs = 0.5208 MHz / (16*2) ~= 16.276 KHz
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//I2S audio-related
reg mclk; //I2S master clock
reg[5:0] mclk_cnt; //I2S master clock
reg [8:0] audio_cnt;
reg [3:0] lrcnt;
reg sdata;  //I2S data input
reg sclk;   //I2S sclk
reg lrclk;  //I2S left-right clock

assign AUDIO_MCLK = mclk;
assign AUDIO_SDIN = sdata;
assign AUDIO_SCLK = sclk;
assign AUDIO_LRCLK = lrclk;

reg signed [15:0] audio_sample;
reg signed [15:0] left_channel;
reg signed [15:0] right_channel;
reg signed [15:0] audio_ctrl=0;


//16K x 8 BRAM
reg [15:0] rdata;
reg [15:0] wdata;
reg [9:0] bram_raddr;
reg [9:0] bram_waddr;
reg[1:0] bram_lat;


//let yosys infer the ram type/configuration 
bram_256_16 bram_rw_fsmc (
  .clk(clk ),
  .ren(1'b1),
  .wen(bram_lat[1]),
  .raddr(bram_raddr[9:0]),
  .waddr(bram_waddr[9:0]),
  .wdata(wdata),
  .rdata(rdata)
);

assign A0 = bram_raddr[9];  //audio interrupt. The msb signals a rising/falling edge
                            //interrupt on the MCU letting it know it is time to
                            //fill half the audio fifo buffer

reg [6:0] mic_cnt;
reg [15:0] mic_ones;
reg [15:0] mic_zero;
reg [15:0] mic_pcm_unflt;

always @(posedge clk) begin

 if(!rst_t) 
 begin
  led2 <= 1'b1; 
  audio_sample <= 16'd0;
  left_channel <= 16'd0;
  right_channel <= 16'd0;
 end else begin
     //left_channel <= rand16_1;
     //right_channel <= rand16_1;

    
  /*
     mic_cnt <= mic_cnt+1;
     if( mic_cnt == 7'd48 ) begin
      mic_cnt <= 7'd0;
      mic_clk <= ~mic_clk; 

      mic_pcm_unflt <= 
     end

   if( mic_clk==1'b1 && mic_cnt == 7'd11) 
   begin
     if( mic_data==1'b1 ) 
       mic_ones <= mic_ones+1;
     else 
       mic_zero <= mic_zero+1;
   end
  */


    mclk_cnt <= mclk_cnt+1;
    //if( mclk_cnt == 6'd3 ) 
    if( mclk_cnt == 6'd11 ) 
    begin
      mclk_cnt <= 6'd0;
      mclk <= ~mclk; 
    end

    audio_cnt <= audio_cnt+1;

    //if( audio_cnt==8'd63 ) //time to toggle I2S sclk
    if( audio_cnt==8'd191 ) //time to toggle I2S sclk
    begin  

      audio_cnt <= 8'd0;

      sclk <= ~sclk; 
      if(sclk) 
      begin

        if(lrcnt==4'd0) //time to output frame an audio sample for left or right
        begin 
          lrclk <= ~lrclk;

          led2 <= lrclk; 
          led2 <= ~audio_sample[0]; 


          if(lrclk) 
            audio_sample <= {1'b0,left_channel[15:1]};
          else 
            audio_sample <= {1'b0,right_channel[15:1]};

        end

        if( lrcnt==4'd1 ) bram_raddr <= bram_raddr + 1;

        if( lrcnt==4'd2 ) 
        begin
          left_channel <= rdata[15:0];
          right_channel <= rdata[15:0];
        end


        //MSB first to I2S dac
        sdata <= audio_sample[15-lrcnt];

        lrcnt <= lrcnt+1;

      end 

    end
 end
end

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Stream I/Q data from fifo to SDR TX at via ddr_out
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
reg [2:0] rx_lsb0; 
reg [2:0] rx_lsb1; 

reg [12:0] rf900_i_data;
reg [12:0] rf900_q_data;
wire rf900_iq_sync;
wire rf900_ddr_datavalid;
wire rx_data_900;
assign rx_data_900 = FPGA_RX900; 

reg [1:0] iq900_dv;
reg [8:0] iq900_addr;

reg [12:0] tx_data_i;
reg [12:0] tx_data_q;
reg tx_next_data;


reg do_tx_eec;

reg tx_clk_n;
reg tx_data_n;


assign FPGA_TXCLK = !tx_clk_n;
assign FPGA_TX_24_900 = !tx_data_n;

reg [3:0] tx_sample_rate_t = 4'ha;  //default sample rate of 400khz 
//read-only signals to be memory-mapped on mcu
reg [15:0] I_iq900_fifo_data_r1;
reg [15:0] Q_iq900_fifo_data_r1;
reg [15:0] I_iq900_fifo_data_r2;
reg [15:0] Q_iq900_fifo_data_r2;
reg [15:0] rf900_data_i16;
reg [15:0] rf900_data_q16;

//tx buffer signals
reg [15:0] IQ_iq900_fifo_data_t1;
reg [15:0] IQ_iq900_fifo_data_t2;

reg [15:0] tx_rf900_data_iq16_f1; //fifo 1
reg [15:0] tx_rf900_data_iq16_f2; //fifo 2

reg [8:0] tx_mem_addr;



always @(negedge tx_next_data) begin


  if(!do_tx_eec) 
  begin
    tx_mem_addr <= 9'b0;
    tx_data_i[12:0] <= 13'b0;
    tx_data_q[12:0] <= 13'b0;
  end 
  else 
  begin

    if(tx_mem_addr[8]==1'b0) 
    begin
      tx_data_i[12:0] <= { IQ_iq900_fifo_data_t1[15:11],tx_i_offset[7:0]}; //add some random bits in the LSB to help spread spurious
      tx_data_q[12:0] <= { IQ_iq900_fifo_data_t1[7:3], tx_q_offset[7:0]};
    end 
    else 
    begin
      tx_data_i[12:0] <= { IQ_iq900_fifo_data_t2[15:11],tx_i_offset[7:0]};
      tx_data_q[12:0] <= { IQ_iq900_fifo_data_t2[7:3], tx_q_offset[7:0]};
    end

    tx_mem_addr <= tx_mem_addr+1;
  end
end

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Stream I/Q data to fifo from SDR RX via ddr_in
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

reg cic_i_dvalid;
reg cic_q_dvalid;
reg [1:0] cic_valid_edge;

cic_filter #(.width(23)) cic_i (
  .clk(iq900_dv[1:0]==2'b01),
  .rst(!rst_t),
  .decimation_ratio(4),
  .d_in(rf900_i_data[12:5]),
  .d_out(rf900_data_i16[15:8]),
  .d_clk(cic_i_dvalid)
);

cic_filter #(.width(23)) cic_q (
  .clk(iq900_dv[1:0]==2'b01),
  .rst(!rst_t),
  .decimation_ratio(4),
  .d_in(rf900_q_data[12:5]),
  .d_out(rf900_data_q16[15:8]),
  .d_clk(cic_q_dvalid)
);

always @(posedge rx_clk) begin

    //detect rising-edge of IQ stream, falling edge latches into fifo memory
    iq900_dv <= { iq900_dv[0], rf900_ddr_datavalid };

    rx_lsb0[2:0] <= 3'b000; 
    rx_lsb1[2:0] <= 3'b111; 


    if( iq900_dv[1:0] == 2'b01 ) 
    begin
      rf900_data_i16[7:0] <= 8'd0;
      rf900_data_q16[7:0] <= 8'd0;
    end 

    cic_valid_edge[1:0] = { cic_valid_edge[0], cic_i_dvalid };

    if(cic_valid_edge[1:0]==2'b10) iq900_addr <= iq900_addr+1;

end

endmodule
