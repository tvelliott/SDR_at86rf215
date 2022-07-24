
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



#include "stm32h7xx_hal.h"

#include <math.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include "globals.h"
#include "fpga_image.h"
#include "fpga.h"
#include "std_io.h"
#include "rle.h"
#include "lwip.h"
#include "p25/p25_decode.h"

volatile float angle;

volatile int low_side;
volatile uint16_t audio_fifo_addr_offset;

static uint16_t volatile *audio_ptr = ( void * ) 0x6000c000;
static uint16_t volatile *audio_addr_ptr = ( void * ) 0x6000d000;

static const uint16_t volatile *rnd_ptr = ( void * ) 0x6000f000;
static const uint16_t volatile *const1_ptr = ( void * ) 0x60001000;
static const uint16_t volatile *const2_ptr = ( void * ) 0x60002000;
static uint16_t volatile *rw_ptr = ( void * ) 0x60007000;


static uint16_t volatile test_val1;
static uint16_t volatile test_val2;
static uint16_t volatile test_val3;
static uint16_t volatile test_val4;

static SRAM_HandleTypeDef hsram1; //FSMC config

void MX_FSMC_Init( void );


extern volatile int uptime_seconds;

volatile int ne4_count;

volatile uint8_t tx_fifo0_900_ready;
volatile uint8_t rx_fifo0_900_ready;
volatile uint8_t fifo0_24_ready;
volatile uint8_t exti4_glitch;


static int32_t prevChar = RLE_NULL;
static int32_t currChar;


static uint8_t byte_in;
static int8_t *in_ptr;
static int out_bytes;
static int ii;
static uint8_t count;
int fpga_bytes_out;

volatile int16_t rval;


volatile int8_t do_handle_exti0;
volatile int8_t do_handle_exti1;
volatile int8_t do_handle_exti4;
volatile int8_t do_handle_exti15_10;



//PORTG
#define DISP_RS GPIO_PIN_0
#define DISP_SI GPIO_PIN_1
#define DISP_CSB GPIO_PIN_2
#define DISP_SCL GPIO_PIN_3
#define DISP_RST GPIO_PIN_4

int disp_init = 0;

/*
///////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////
void test_display(void) {

  if(disp_init) return;

  disp_write_com(0x30);

  HAL_GPIO_WritePin( GPIOG, DISP_RST, 0 );
  HAL_Delay(2);
  HAL_GPIO_WritePin( GPIOG, DISP_RST, 1 );
  HAL_Delay(20);

  disp_write_com(0x30);
  HAL_Delay(2);
  disp_write_com(0x30);
  disp_write_com(0x30);
  disp_write_com(0x39);
  disp_write_com(0x14);
  disp_write_com(0x56);
  disp_write_com(0x6d);
  disp_write_com(0x70);
  disp_write_com(0x0c);
  disp_write_com(0x06);
  disp_write_com(0x01);
  HAL_Delay(10);

  //disp_write_com(0x01); //clear display
  //disp_write_com(0x02); //return home
  //disp_write_com(0x0f); //blinking cursor

  disp_write_data('T');
  disp_write_data('E');
  disp_write_data('S');
  disp_write_data('T');

  disp_init=1;
}

///////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////
void disp_write_com(uint8_t val) {
  int i;
  uint8_t d = val;

  HAL_GPIO_WritePin( GPIOG, DISP_CSB, 0 );
  HAL_GPIO_WritePin( GPIOG, DISP_RS, 0 );

  for(i=0;i<8;i++) {
    delay_us(2);

    if( (d&0x80)==0x80 ) HAL_GPIO_WritePin( GPIOG, DISP_SI, 1 );
      else HAL_GPIO_WritePin( GPIOG, DISP_SI, 0 );
    d <<= 1;

    HAL_GPIO_WritePin( GPIOG, DISP_SCL, 0 );
    delay_us(2);
    HAL_GPIO_WritePin( GPIOG, DISP_SCL, 1 );
    delay_us(2);
    HAL_GPIO_WritePin( GPIOG, DISP_SCL, 0 );
    delay_us(2);
  }

  HAL_GPIO_WritePin( GPIOG, DISP_CSB, 1 );
  delay_us(2);
}
///////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////
void disp_write_data(uint8_t val) {
  int i;
  uint8_t d = val;

  HAL_GPIO_WritePin( GPIOG, DISP_CSB, 0 );
  HAL_GPIO_WritePin( GPIOG, DISP_RS, 1 );

  for(i=0;i<8;i++) {
    delay_us(2);

    if( (d&0x80)==0x80 ) HAL_GPIO_WritePin( GPIOG, DISP_SI, 1 );
      else HAL_GPIO_WritePin( GPIOG, DISP_SI, 0 );
    d <<= 1;

    HAL_GPIO_WritePin( GPIOG, DISP_SCL, 0 );
    delay_us(2);
    HAL_GPIO_WritePin( GPIOG, DISP_SCL, 1 );
    delay_us(2);
    HAL_GPIO_WritePin( GPIOG, DISP_SCL, 0 );
    delay_us(2);
  }

  HAL_GPIO_WritePin( GPIOG, DISP_CSB, 1 );
  delay_us(2);
}

///////////////////////////////////////////////////////////////
// PORTF 2,3,4,5, 12,13
///////////////////////////////////////////////////////////////
void show_button_reg(void) {
  uint8_t buttons;
  buttons = 0x00;
  buttons <<=1;

  buttons |= ( HAL_GPIO_ReadPin( GPIOF, GPIO_PIN_2 ) & 0x01 );
  buttons <<=1;
  buttons |= ( HAL_GPIO_ReadPin( GPIOF, GPIO_PIN_3 ) & 0x01 );
  buttons <<=1;
  buttons |= ( HAL_GPIO_ReadPin( GPIOF, GPIO_PIN_4 ) & 0x01 );
  buttons <<=1;
  buttons |= ( HAL_GPIO_ReadPin( GPIOF, GPIO_PIN_5 ) & 0x01 );
  buttons <<=1;
  buttons |= ( HAL_GPIO_ReadPin( GPIOF, GPIO_PIN_12 ) & 0x01 );
  buttons <<=1;
  buttons |= ( HAL_GPIO_ReadPin( GPIOF, GPIO_PIN_13 ) & 0x01 );


  //printf("\r\nbuttons: %02x", buttons);

  //test_display();
}
*/

///////////////////////////////////////////////////////////////
// configure GPIOF PIN 0 as audio interrupt for fifo fill
// was ALT function of A0 for address bus.  not used as A0
// since bus is multiplexed
///////////////////////////////////////////////////////////////
void audio_int_config( void )
{
  GPIO_InitTypeDef   GPIO_InitStructure;

  // Enable GPIOF clock
  __HAL_RCC_GPIOF_CLK_ENABLE();

  GPIO_InitStructure.Mode = GPIO_MODE_IT_RISING_FALLING;
  GPIO_InitStructure.Pull = GPIO_NOPULL;
  GPIO_InitStructure.Pin = GPIO_PIN_0;
  HAL_GPIO_Init( GPIOF, &GPIO_InitStructure );

  // Enable and set EXTI lines 0 Interrupt priority
  HAL_NVIC_SetPriority( EXTI0_IRQn, 1, 1 ); //audio fifo fill interrupt priority
  HAL_NVIC_EnableIRQ( EXTI0_IRQn );
}


/////////////////////////////////////////////////////////////////////////////////////////
// put the fpga into spi slave / programming mode
/////////////////////////////////////////////////////////////////////////////////////////
void fpga_init()
{
  GPIO_InitTypeDef GPIO_InitStruct;

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOG_CLK_ENABLE();



  GPIO_InitStruct.Pin = GPIO_PIN_6 | GPIO_PIN_8;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init( GPIOG, &GPIO_InitStruct );

  HAL_GPIO_WritePin( GPIOG, GPIO_PIN_8, GPIO_PIN_RESET ); //FPGA CRESET_B


  GPIO_InitStruct.Pin = GPIO_PIN_7 | GPIO_PIN_10 | GPIO_PIN_12;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init( GPIOC, &GPIO_InitStruct );

  GPIO_InitStruct.Pin = GPIO_PIN_11;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init( GPIOC, &GPIO_InitStruct );

  GPIO_InitStruct.Pin = GPIO_PIN_7;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init( GPIOG, &GPIO_InitStruct );


  GPIO_InitStruct.Pin = GPIO_PIN_14;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init( GPIOB, &GPIO_InitStruct );

  HAL_GPIO_WritePin( GPIOB, GPIO_PIN_14, GPIO_PIN_SET );  //FPGA spi3 cs
  HAL_GPIO_WritePin( GPIOC, GPIO_PIN_7, GPIO_PIN_RESET ); //FPGA spi_cs
  HAL_GPIO_WritePin( GPIOC, GPIO_PIN_10, GPIO_PIN_SET );  //FPGA spi_sck

  HAL_Delay( 500 ); //500 ms

  HAL_GPIO_WritePin( GPIOG, GPIO_PIN_8, GPIO_PIN_SET ); //FPGA CRESET_B

  HAL_Delay( 500 ); //500 ms

  //fpga config size for 8k lattice device is constant 135100
  fpga_rle_decode( rle_compressed_fpga_image, 135100 );

  HAL_GPIO_WritePin( GPIOC, GPIO_PIN_7, GPIO_PIN_SET ); //FPGA spi_cs

  //fpga !rst_t signal
  HAL_GPIO_WritePin( GPIOG, GPIO_PIN_6, GPIO_PIN_RESET ); //FPGA CBSEL
  HAL_Delay( 50 ); //50 ms
  HAL_GPIO_WritePin( GPIOG, GPIO_PIN_6, GPIO_PIN_SET ); //FPGA CBSEL  fpga should be ready
  HAL_Delay( 150 ); //150 ms

  printf( "\r\ncdone: %d", HAL_GPIO_ReadPin( GPIOG, GPIO_PIN_7 ) );

}


//////////////////////////////////////////////////////////////////////////
// called from EXTI0_IRQHandler in st_xxx_it.c
// execution time of this should be as short as possible.
//////////////////////////////////////////////////////////////////////////
void do_audio_fifo_fill()
{

  int vi;

  for( vi = 0; vi < 512; vi++ ) {

    if( is_playing && out_s != out_e && config->audio_on ) {
      *audio_ptr = out_buffer[out_s++];
      out_s &= OUT_BUFFER_SIZE - 1;
    } else {
#if 1
      *audio_ptr = 0x0000;   //silence
      //if(out_s==out_e) is_playing=0;
#else
      *audio_ptr = ( int16_t )( sinf( angle ) * 32767.0f ); //test ~2khz sine wave output
      angle += 0.75;
#endif
    }

    *audio_addr_ptr = audio_fifo_addr_offset++;

  }
}


//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
void do_tx_eec_off( void )
{
  //GPIO_ResetBits(GPIOF, GPIO_Pin_3);  //DONT USE PORTF
}
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
void do_tx_eec_on( void )
{
  //GPIO_SetBits(GPIOF, GPIO_Pin_3);    //DONT USE PORTF
}


//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
void mcu_led_on( void )
{
}
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
void mcu_led_off( void )
{
}

//////////////////////////////////////////////////////////////////////////
// bit-bang configuration of fpga
//////////////////////////////////////////////////////////////////////////
static uint8_t write_fpga( uint8_t out_reg )
{
  uint8_t in_reg = 0;
  uint8_t out_byte = out_reg;

  for( ii = 0; ii < 8; ii++ ) {
    in_reg <<= 1;

    //setup mosi
    if( out_byte & 0x80 ) HAL_GPIO_WritePin( GPIOC, GPIO_PIN_12, GPIO_PIN_SET ); //MOSI
    else HAL_GPIO_WritePin( GPIOC, GPIO_PIN_12, GPIO_PIN_RESET ); //MOSI

    out_byte <<= 1;

    //sclk low
    HAL_GPIO_WritePin( GPIOC, GPIO_PIN_10, GPIO_PIN_RESET ); //SCLK

    if( HAL_GPIO_ReadPin( GPIOC, GPIO_PIN_11 ) == GPIO_PIN_SET ) in_reg |= 0x01;

    //sclk high.  data is clocked in on rising edge of sclk
    HAL_GPIO_WritePin( GPIOC, GPIO_PIN_10, GPIO_PIN_SET ); //SCLK

  }

  fpga_bytes_out++;
  return in_reg;
}

//////////////////////////////////////////////////////////////////////////////////////////////
// uncompress the run-length encoded fpga image and send to fpga
//////////////////////////////////////////////////////////////////////////////////////////////
uint8_t fpga_rle_decode( const uint8_t *image, int len )
{
  in_ptr = ( int8_t * ) image;
  out_bytes = 0;
  fpga_bytes_out = 0;

  prevChar = RLE_NULL;

  while( out_bytes < len + 1 ) {

    byte_in = *in_ptr++;
    currChar = byte_in;

    //write byte_in to fpga
    write_fpga( byte_in );

    out_bytes++;

    if( currChar == prevChar ) {

      byte_in = *in_ptr++;
      count = byte_in;

      while( count > 0 ) {
        //write byte_in to fpga = (uint8_t) (currChar&0xff);
        write_fpga( ( uint8_t )( currChar & 0xff ) );

        out_bytes++;
        count--;
      }

      prevChar = RLE_NULL;
    } else {
      prevChar = currChar;
    }
  }


  //Lattice datasheet says keep clocking for 64 more cycles
  write_fpga( 0xff );
  write_fpga( 0xff );
  write_fpga( 0xff );
  write_fpga( 0xff );
  write_fpga( 0xff );
  write_fpga( 0xff );
  write_fpga( 0xff );
  write_fpga( 0xff );

  return out_bytes - 1;
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
void set_ioffset( uint16_t val )
{
  //uint16_t *ptr_wo_config;
  /*
  ptr_wo_config = 0x60004006;
    *ptr_wo_config = val;  //i-offset, 5-lsb
  */
}
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
void set_qoffset( uint16_t val )
{
  //uint16_t *ptr_wo_config;
  /*
    ptr_wo_config = 0x60004008;
    *ptr_wo_config = val;  //q-offset, 5-lsb
  */
}
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
void dump_fifo()
{

  /*

  //bram on fpga
  int16_t *ptr_i1 = 0x60004000;
  int16_t *ptr_q1 = 0x60005000;
  int16_t *ptr_i2 = 0x60006000;
  int16_t *ptr_q2 = 0x60007800;

  int16_t idata1[256];
  int16_t qdata1[256];
  int16_t idata2[256];
  int16_t qdata2[256];

  for(ii=0; ii<256; ii++) {
    idata1[ii] = *ptr_i1++;
    qdata1[ii] = *ptr_q1++;
    idata2[ii] = *ptr_i2++;
    qdata2[ii] = *ptr_q2++;
  }
  for(ii=0; ii<256; ii++) {
    printf("\r\n%d,%d", idata1[ii], qdata1[ii]);
  }

  printf("\r\n ");

  for(ii=0; ii<256; ii++) {
    printf("\r\n%d,%d", idata2[ii], qdata2[ii]);
  }

  print_prompt();
  */
}
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
void test_mem()
{



  int i;
  int errors = 0;

  //SCB_DisableDCache();
  //SCB_DisableICache();

  *audio_addr_ptr = 0x0000;
  uint16_t volatile *ptr1;

  DelayClk3( 150 );
  ptr1 = ( uint16_t * ) rw_ptr;

#if 1
  for( i = 0; i < 65535; i++ ) {

    *ptr1 = ( uint16_t ) i;

    test_val1 = *rnd_ptr;
    test_val2 = *const1_ptr;
    test_val4 = *const2_ptr;


    //DelayClk3(150);

#if 1
    if( i % 8192 == 0 ) {
      printf( "\r\nrnd: %04x", test_val1 );
      //printf("\r\nrconst 0x5555: 0x%04x", test_val2);
      //printf("\r\nrconst 0xaaaa: 0x%04x", test_val4);
      printf( "\r\nerrors: %d of %d, last val 0x%04x", errors, i, test_val3 );
      main_tick();
    }

    test_val3 = *ptr1;

    ptr1 = rw_ptr + ( ( i + 1 ) * 2 ) % 256;
    //DelayClk3(150);

#endif


    if( test_val3 != ( uint16_t ) i ) {
      //printf("\r\nerror: rw: %04x: %04x", i, test_val3);
      //goto exit_test_mem;
      errors++;
    }
    if( test_val2 != 0x5555 ) {
      printf( "\r\nerror: const: 0x5555: %04x", test_val2 );
      goto exit_test_mem;
    }
    if( test_val4 != 0xAAAA ) {
      printf( "\r\nerror: const: 0xAAAA: %04x", test_val4 );
      goto exit_test_mem;
    }


    //HAL_Delay(1);
  }
#endif


  ptr1 = ( uint16_t * ) rw_ptr;
  *audio_addr_ptr = 0;
  //DelayClk3( 150 );


#if 1
  for( i = 0; i < 2048 * 2048; i++ ) {
    *audio_addr_ptr = i;
    DelayClk3( 15 );
    *ptr1 = rand() % 65535;
    DelayClk3( 15 );

    //printf( "\r\nval %d: %04x", i, *ptr1 );  //0xc000
  }
#endif

#if 0
  ptr1 = ( uint16_t * ) rw_ptr;
  *audio_addr_ptr = 0;
  //DelayClk3( 150 );

  for( i = 0; i < 32; i++ ) {
    //DelayClk3( 150 );
    printf( "\r\nval %d: %04x", i, *ptr1 );  //0xc000
    *audio_addr_ptr = ( i + 1 ) * 2;
  }
#endif

#if 0
  printf( "\r\nstarting tfer 10000000" );
  uint32_t stime = HAL_GetTick();
  for( i = 0; i < 10000000; i++ ) {
    *audio_addr_ptr = i % 2048;
    *ptr1 = i;  //0xc000
    test_val1 = *ptr1;
  }
  uint32_t etime = HAL_GetTick();
  printf( "\r\n%d ms", etime - stime );
#endif
  if( errors == 0 ) printf( "\r\nfinished ok" );
  else printf( "\r\nfinished with %d errors", errors );

exit_test_mem:
  //SCB_EnableDCache();
  //SCB_EnableICache();

  return;
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
void EXTI4_IRQHandler()
{
  /*
  EXTI_ClearITPendingBit(EXTI_Line4);

  //if(do_handle_exti4) exti4_glitch=1;

  tx_fifo0_900_ready = GPIO_ReadInputDataBit(GPIOF, GPIO_Pin_4);  //DONT USE PORTF
  //do_handle_exti4=1;
  //if(ne4_count++%8000==0) printf("\r\nexti4 count: %d", ne4_count);
  fill_tx_fifo();
  //check_interpolate();
  */
}


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
void EXTI15_10_IRQHandler()
{
  /*
  EXTI_ClearITPendingBit(EXTI_Line12);
  do_handle_exti15_10=1;
  */
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
void _handle_exti15_10()
{
}



////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MX_FSMC_Init( void )
{

  FMC_NORSRAM_TimingTypeDef Timing;

  /** Perform the SRAM1 memory initialization sequence
  */
  hsram1.Instance = FMC_NORSRAM_DEVICE;
  hsram1.Extended = FMC_NORSRAM_EXTENDED_DEVICE;
  /* hsram1.Init */
  hsram1.Init.NSBank = FMC_NORSRAM_BANK1;
  hsram1.Init.DataAddressMux = FMC_DATA_ADDRESS_MUX_ENABLE;
  hsram1.Init.MemoryType = FMC_MEMORY_TYPE_PSRAM;
  hsram1.Init.MemoryDataWidth = FMC_NORSRAM_MEM_BUS_WIDTH_16;
  hsram1.Init.BurstAccessMode = FMC_BURST_ACCESS_MODE_ENABLE;
  hsram1.Init.WaitSignalPolarity = FMC_WAIT_SIGNAL_POLARITY_LOW;
  hsram1.Init.WaitSignalActive = FMC_WAIT_TIMING_BEFORE_WS;
  hsram1.Init.WriteOperation = FMC_WRITE_OPERATION_ENABLE;
  hsram1.Init.WaitSignal = FMC_WAIT_SIGNAL_DISABLE;
  hsram1.Init.ExtendedMode = FMC_EXTENDED_MODE_DISABLE;
  hsram1.Init.AsynchronousWait = FMC_ASYNCHRONOUS_WAIT_DISABLE;
  hsram1.Init.WriteBurst = FMC_WRITE_BURST_ENABLE;
  hsram1.Init.ContinuousClock = FMC_CONTINUOUS_CLOCK_SYNC_ASYNC;
  hsram1.Init.WriteFifo = FMC_WRITE_FIFO_ENABLE;
  hsram1.Init.PageSize = FMC_PAGE_SIZE_NONE;

  /* Timing */
  Timing.AddressSetupTime = 1;
  Timing.AddressHoldTime = 1;
  Timing.DataSetupTime = 1;


  Timing.BusTurnAroundDuration = 1;


  Timing.CLKDivision = 2;
  Timing.DataLatency = 6; //with this 6, then nwe_count in xx_top.v should be 4
  Timing.AccessMode = FMC_ACCESS_MODE_A;
  /* ExtTiming */

  if( HAL_SRAM_Init( &hsram1, &Timing, NULL ) != HAL_OK ) {
    _Error_Handler( __FILE__, __LINE__ );
  }

}
