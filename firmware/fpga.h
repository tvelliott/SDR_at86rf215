
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



#ifndef __FPGA_H__
#define __FPGA_H__

#define BTR_REG_RW_FAST  0x01110000;
#define BTR_BRAM_RW_FAST 0x00200000;

extern volatile int low_side;
extern volatile uint16_t audio_fifo_addr_offset;
#define AUDIO_BUFFER_SIZE 1024

void MX_FSMC_Init( void );
void enable_mco2_sysclk();
void init_mclk();
void _handle_exti15_10();
void EXTI15_10_IRQHandler();
void EXTI4_IRQHandler();
void EXTI1_IRQHandler();
void test_mem();
void dump_fifo();
void set_qoffset( uint16_t val );
void set_ioffset( uint16_t val );
void do_audio_fifo_fill( void );
void mcu_led_off( void );
void mcu_led_on( void );
void do_tx_eec_on( void );
void do_tx_eec_off( void );
uint8_t fpga_rle_decode( const uint8_t *image, int len );
void fpga_init( void );
void test_display( void );
void disp_write_com( uint8_t val );
void disp_write_data( uint8_t val );
extern volatile int8_t do_handle_exti15_10;
extern volatile int8_t do_handle_exti4;
extern volatile int8_t do_handle_exti0;
extern volatile int16_t rval;
extern int fpga_bytes_out;
extern volatile uint8_t exti4_glitch;
extern volatile uint8_t fifo0_24_ready;
extern volatile uint8_t rx_fifo0_900_ready;
extern volatile uint8_t tx_fifo0_900_ready;
extern volatile int ne4_count;
extern volatile int uptime_seconds;
#endif
