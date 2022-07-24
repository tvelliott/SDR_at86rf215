

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


#include "main.h"
#include "usbd_cdc_core.h"
#include "usbd_usr.h"
#include "usbd_desc.h"
#include "usbd_cdc_vcp.h"

#include "stm32f4xx_flash.h"
#include "stm32f4xx_spi.h"
#include "stm32f4_discovery.h"

#include "global.h"
#include "std_io.h"
#include "eth_hal_nxp_ngx.h"
#include "telnet.h"
#include "crc.h"
#include <math.h>
#include <ctype.h>
#include <stdio.h>
#include "config.h"

#define SPIx                           SPI2
#define SPIx_CLK                       RCC_APB1Periph_SPI2
#define SPIx_CLK_INIT                  RCC_APB1PeriphClockCmd
#define SPIx_IRQn                      SPI2_IRQn
#define SPIx_IRQHANDLER                SPI2_IRQHandler

#define SPIx_SCK_PIN                   GPIO_Pin_13
#define SPIx_SCK_GPIO_PORT             GPIOB
#define SPIx_SCK_GPIO_CLK              RCC_AHB1Periph_GPIOB
#define SPIx_SCK_SOURCE                GPIO_PinSource13
#define SPIx_SCK_AF                    GPIO_AF_SPI2

#define SPIx_MISO_PIN                  GPIO_Pin_14
#define SPIx_MISO_GPIO_PORT            GPIOB
#define SPIx_MISO_GPIO_CLK             RCC_AHB1Periph_GPIOB
#define SPIx_MISO_SOURCE               GPIO_PinSource14
#define SPIx_MISO_AF                   GPIO_AF_SPI2

#define SPIx_MOSI_PIN                  GPIO_Pin_15
#define SPIx_MOSI_GPIO_PORT            GPIOB
#define SPIx_MOSI_GPIO_CLK             RCC_AHB1Periph_GPIOB
#define SPIx_MOSI_SOURCE               GPIO_PinSource15
#define SPIx_MOSI_AF                   GPIO_AF_SPI2

//freq can be from 3MHz to 1090 MHz
//IF passband is 905-925.  images may appear in this range , TODO: should try tuning LO below the filter to push freq up into the IF for >900 MHz

#define FREQ_OFFSET ( 0.002065 )
double ppm = 2.766;


#define TRANSITION_FREQ (500.0)
#define IF_FREQ (908.25)
#define start_freq (float) (931.2125)

void assert_param(int pass);
__ALIGN_BEGIN USB_OTG_CORE_HANDLE  USB_OTG_dev __ALIGN_END;

    uint8_t aTxBuffer[BUFFERSIZE] = "SPI Master/Slave : Communication between two SPI using Interrupts";
volatile uint8_t aRxBuffer [BUFFERSIZE];
volatile uint8_t ubRxIndex = 0;
volatile uint8_t ubTxIndex = 0;
volatile uint32_t TimeOut = 0;
extern volatile uint8_t spi_rx_int;
extern volatile uint8_t rx_len;
volatile int spi2_init=0;

uint8_t spi2_rx(void);
uint8_t spi2_tx_slave(uint8_t byte);

GPIO_InitTypeDef GPIO_InitStruct;
SPI_InitTypeDef SPI_InitStructure;

uint8_t wait_for_lock(void);
void rf_mute(void);
void rf_unmute(void);
void set_freq_mhz(double freq);
void set_freq_hz(uint32_t freq);
void delay_ms(int ms);
uint8_t synth_read_reg(uint8_t addr);
void synth_write_reg(uint8_t addr, uint8_t val);
uint8_t spi1_rx(void);
uint8_t spi1_tx(uint8_t byte);
void init_spi1(void);
void init_spi2(void);
void dump_regs(void);

void do_main_tasks(void);

__IO uint32_t TimingDelay;
__IO uint32_t tick_count;
volatile uint32_t LocalTime;
static int8_t do_dump_constant;
static int8_t do_dump;

volatile int do_freq_change=0;

/* Ethernet Driver Receive buffers  */
extern volatile uint8_t Rx_Buff[];

/* Ethernet Driver Transmit buffers */
extern volatile uint8_t Tx_Buff[];

/////////////////////////////////////////////
// delay for 3 clock cycles * ulCount
/////////////////////////////////////////////
void __attribute__((naked)) DelayClk3(unsigned long ulCount)
{
  __asm("    subs    r0, #1\n"
        "    bne     DelayClk3\n"
        "    bx      lr");
}

void handle_command(int16_t interface);

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void handle_command(int16_t interface)
{
  int32_t count;
  double f1;

  if( strcmp(acm_in_buffer, "dreg") == 0) {
  } else if( strcmp(acm_in_buffer, "init") == 0) {
  } else if( strcmp(acm_in_buffer, "dc") == 0) {
    do_dump_constant ^= 0x01;
  } else if( strcmp(acm_in_buffer, "send") == 0) {
  } else if( strcmp(acm_in_buffer, "dump") == 0) {
    dump_regs();
  } else if( strcmp(acm_in_buffer, "debug") == 0) {
  } else if( strcmp(acm_in_buffer, "crc") == 0) {
    crc_test();
  } else if( strcmp(acm_in_buffer, "writeconfig") == 0) {
    write_config( &config );
  } else if( strcmp(acm_in_buffer, "initsynth") == 0) {
    init_synth(); 
  } else if( strcmp(acm_in_buffer, "readconfig") == 0) {
    read_config( );
  } else if( strcmp(acm_in_buffer, "defaults") == 0) {
    printf("\r\nreset configuration\r\n");
    reset_config_to_defaults( );
  } else if( strcmp(acm_in_buffer, "showconfig") == 0) {
  } else if( strncmp("freq", acm_in_buffer, 4) == 0) {
    acm_in_buffer[4]=0;
    f1 = (double) atof( &acm_in_buffer[5] );
    if(f1 >= 5.0 && f1 <= 1100.0) {
      config.frequency = (double) f1;
      printf("\r\nfrequency: %f\r\n", config.frequency);
      set_freq_mhz( IF_FREQ + config.frequency );
      wait_for_lock();
    }
  } else if( acm_scanf("txt %ld", &count) == 1) {
  } else if( strncmp("thresh", acm_in_buffer, 5) == 0) {
    acm_in_buffer[5]=0;
    f1 = (double) atof( &acm_in_buffer[6] );

    if(f1 >= 5.0 && f1 <= 1100.0) {
      config.thresh_freq = (double) f1;
      printf("\r\nthresh frequency: %f\r\n", config.thresh_freq);
      set_freq_mhz( IF_FREQ + config.frequency );
    }
  }

  uint32_t clock_mhz = SystemCoreClock / 1e6L;
  uint8_t lock = synth_read_reg(0x00);

  char lock_status[32];
  if(lock==4) sprintf(lock_status, "LOCKED @ %f, tuned to %f, status: %02x", config.frequency+IF_FREQ, config.frequency, lock);
    else sprintf(lock_status, "UN_LOCKED!!   status: %02x", lock);

  if(interface==ACM_INTF) {
    printf("\r\nConnected To STM32F405 Running @ %u MHz, synth is %s", (unsigned int) clock_mhz, lock_status); 
    printf( "\r\nVWR>" );

    //set_freq_mhz( start_freq );
  }
}


///////////////////////////////////////////////////////////////////////////////
// REGS are address 0 to 0x1f,   16-bit words, msb first
// a7 bit H=read,  L=write
///////////////////////////////////////////////////////////////////////////////
void dump_regs() {

uint8_t address;
uint8_t reg;

  printf("\r\n ");

  for(address=0;address<0x0f;address++) {
    reg = synth_read_reg(address);
    printf("\r\n%02x: %02x",address, reg); 
  }

  printf( "\r\n\r\nVWR>" );

}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
int main(void)
{
  RCC_ClocksTypeDef RCC_Clocks;

  USBD_Init(&USB_OTG_dev,
            USB_OTG_FS_CORE_ID,
            &USR_desc,
            &USBD_CDC_cb,
            &USR_cb);

  RCC_GetClocksFreq(&RCC_Clocks);
  SysTick_Config(RCC_Clocks.HCLK_Frequency / 50);


  GPIO_InitTypeDef  GPIO_InitStructure;
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);


  GPIO_InitStructure.GPIO_Pin = ANT_SW | MUTE_N | SYNTH_CS;   
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_Init(GPIOA, &GPIO_InitStructure);

  GPIO_SetBits(GPIOA, ANT_SW);   //ANT_SW   HIGH=VHF enable
  GPIO_SetBits(GPIOA, MUTE_N);   //enable synth output 
  GPIO_SetBits(GPIOA, SYNTH_CS);   //synth spi cs high

  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
  GPIO_InitStructure.GPIO_Pin = TUNER_SLAVE_CS; 
  GPIO_Init(GPIOC, &GPIO_InitStructure);

  //enable FPU
  //*((volatile unsigned long*)0xE000ED88) = 0xF << 20;

  read_config();

  //eem_init_state();

  //LwIP_Init();

  //telnet_init();


  init_spi1();



  init_synth();




  //float freq_to_use=915.0;

  init_spi2();

  uint32_t freq_hz;
  int did_cmd;
  uint8_t *ptr;

//#define TUNER_CMD_SET_IF 0x03
//#define TUNER_CMD_GET_IF 0x04
//#define TUNER_CMD_SET_ANT 0x05
//#define TUNER_CMD_GET_ANT 0x06
//#define TUNER_CMD_SET_MUTE 0x08


  while(1) {

    //this is necessary to keep bits lined up from the master side
    if( !GPIO_ReadInputDataBit(GPIOC, TUNER_SLAVE_CS) ) {
      if(!spi2_init) {
        spi2_init=1;
        SPI_Cmd(SPIx, ENABLE);
      }
    }
    else {
      if(spi2_init) {
        spi2_init=0;
        ubRxIndex=0;
        SPI_Cmd(SPIx, DISABLE);
      }
    }

    //5 bytes have been transferred with !cs
    if(spi2_init && spi_rx_int) {

      did_cmd=0;

      switch( aRxBuffer[1] ) {

          case  TUNER_CMD_SET_FREQ  :

            spi2_tx_slave(0x55); //ack 

            memcpy( (char *) &freq_hz, (char *) &aRxBuffer[2], 4);
            set_freq_hz(freq_hz);
            did_cmd=1;

          break;

          case  TUNER_CMD_GET_LOW_FREQ  :
            freq_hz = 5E6; 
            ptr = (uint8_t *) &freq_hz;

            spi2_tx_slave(*ptr++); 
            spi2_tx_slave(*ptr++); 
            spi2_tx_slave(*ptr++); 
            spi2_tx_slave(*ptr++); 

            did_cmd=1;
          break;

          case  TUNER_CMD_GET_HIGH_FREQ  :
            freq_hz = 900E6; 
            ptr = (uint8_t *) &freq_hz;

            spi2_tx_slave(*ptr++); 
            spi2_tx_slave(*ptr++); 
            spi2_tx_slave(*ptr++); 
            spi2_tx_slave(*ptr++); 

            did_cmd=1;
          break;

          case  TUNER_CMD_GET_LOW_IF  :
            freq_hz = 908250000; 
            ptr = (uint8_t *) &freq_hz;

            spi2_tx_slave(*ptr++); 
            spi2_tx_slave(*ptr++); 
            spi2_tx_slave(*ptr++); 
            spi2_tx_slave(*ptr++); 

            did_cmd=1;
          break;

          case  TUNER_CMD_GET_HIGH_IF  :
            freq_hz = 921750000; 
            ptr = (uint8_t *) &freq_hz;

            spi2_tx_slave(*ptr++); 
            spi2_tx_slave(*ptr++); 
            spi2_tx_slave(*ptr++); 
            spi2_tx_slave(*ptr++); 

            did_cmd=1;
          break;

          case  TUNER_CMD_GET_FREQ  :
            freq_hz = ((uint32_t) config.frequency) * 1E6;
            ptr = (uint8_t *) &freq_hz;

            spi2_tx_slave(*ptr++); 
            spi2_tx_slave(*ptr++); 
            spi2_tx_slave(*ptr++); 
            spi2_tx_slave(*ptr++); 

            did_cmd=1;
          break;

          case  TUNER_CMD_PLL_STATUS  :
            spi2_tx_slave( synth_read_reg(0x00) );
            did_cmd=1;
          break;

      }


      #if 0
      int i;

      if(did_cmd) {
        printf("\r\nspi2 rx ");
        for(i=0;i<rx_len;i++) {
          printf("%02x ", aRxBuffer[i]);
          aRxBuffer[i]=0x00;
        }
        printf("\r\n ");
      }
      #else
        if(did_cmd) {
          ubRxIndex=0;
          memset((char *) aRxBuffer,0x00,255);
        }
      #endif
      //keep these at end
      spi_rx_int=0;

    }


    do_main_tasks();

  } //while

}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void init_synth() {
  synth_write_reg( 0x02, 0x07); //power-on reset
  synth_write_reg( 0x02, 0x06); //default
  rf_mute();
  delay_ms(10);

  synth_write_reg( 0x0d, 0xc0);   //allow the loop to lock

  //synth_write_reg( 0x03, 0x3f); //integer-N mode,  default = 0x3e

  synth_write_reg( 0x04, 0xbe); //LDOV = 2

  synth_write_reg( 0x06, 0x10); //ref divider = 2
  synth_write_reg( 0x07, 127); //N value

    //synth_write_reg( 0x06, 0x41); //ref divider = 8
    //synth_write_reg( 0x07, 0x50); //N value


  synth_write_reg( 0x08, 0x1f); //MSB NUM
  synth_write_reg( 0x09, 0x7f); //write default value to force cal

  //synth_write_reg( 0x0b, 0xf9); //default
  //synth_write_reg( 0x0b, 0x9e); //output divider = 6 
  synth_write_reg( 0x0b, 0x1e); //output divider = 6 

  synth_write_reg( 0x0c, 0x0d);   

  set_freq_mhz( IF_FREQ + config.frequency );

  delay_ms(10);

  rf_unmute();
};
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
uint8_t wait_for_lock(void) {
  uint8_t lock;
  int cnt=100;
  while(--cnt>0) {
    lock = synth_read_reg(0x00);
    if(lock==4) return 0x01; 
    delay_ms(1);
  }

  //if(cnt==0) printf("\r\npll did not lock!!");

  return 0x00;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void set_freq_hz(uint32_t freq) {
  double f = ((double) freq)/1e6;
  config.frequency = f;

  set_freq_mhz(IF_FREQ+f);
}

///////////////////////////////////////////////////////////////////////////////
//  ODIV 1-6
//  R = 1-31
//  F = 1 - 262143   (2^18-1)
//  N = 32-511
///////////////////////////////////////////////////////////////////////////////
void set_freq_mhz(double freq) {
  double REF = 100.0 / 2.0;  //ref div of 2 

  double odiv=1.0;

  //double f1 = freq*ppm;
  //double f2 = f1/1000000.0;
  //freq = freq+f2; 
  //printf("\r\nfreq: %f", freq);

  freq = freq + FREQ_OFFSET;

  //rf_mute();

  if( freq > (IF_FREQ + config.thresh_freq) ) GPIO_ResetBits(GPIOA, ANT_SW);   //ANT_SW   LOW=UHF enable
    else GPIO_SetBits(GPIOA, ANT_SW);   //ANT_SW   HIGH=VHF enable

  //prefer highest output divider possible
  if(freq > 700.0 && freq <= 1065.0) {
    odiv = 6;
  }
  else if(freq > 1065.0 && freq <= 1278.0) {
    odiv = 5;
  }
  else if(freq > 1278.0 && freq <= 1598.0) {
    odiv = 4;
  }
  else if(freq > 1598.0 && freq <= 2130.0) {
    odiv = 3;
  }
  else if(freq > 2130.0 && freq <= 3195.0) {
    odiv = 2;
  }
  else if(freq > 4200.0 && freq <= 6390.0) {
    odiv = 1;
  }

  double fvco = freq*odiv;

  double N = 1.0;

  while( (fvco / N) > REF ) {
    N += 1.0;
    if(N >= 511) break;
  }

  //((fvco*R)/fref)-N = F;

  double freq_int = N*REF;
  double freq_err = (freq_int-fvco);
  double F = 262143 - ((freq_err/REF) * 262143);

  //if(freq_err!=0) N-=1.0;
  N-=1.0;

  uint32_t FINT = (int) F;

  uint32_t F1 = FINT>>12;
  uint32_t F2 = ((FINT>>4)&0xff);
  uint32_t F3 = (FINT&0x0f)<<4;

  uint8_t reg08 = synth_read_reg(0x08);
  uint8_t reg09 = synth_read_reg(0x09);
  uint8_t reg0a = synth_read_reg(0x0a);

  reg08 = F1;

  reg09 = F2;

  reg0a &=0x0f;
  reg0a |=F3;

  synth_write_reg(0x08, reg08);
  synth_write_reg(0x09, reg09);
  synth_write_reg(0x0a, reg0a);

  uint32_t NINT = (int) N;
  uint32_t N1 = NINT>>8;
  uint32_t N2 = (NINT&0xff);

  uint8_t reg06 = synth_read_reg(0x06);
  reg06 &= 0xfc;
  reg06 |= (uint8_t) N1;

  uint8_t reg07 = (uint8_t) N2;

  synth_write_reg( 0x06, reg06); //N value
  synth_write_reg( 0x07, reg07); //N value

  uint8_t reg0b = synth_read_reg(0x0b);
  reg0b &= 0xf8;
  reg0b |= (uint8_t) odiv;
  synth_write_reg( 0x0b, reg0b); //ODIV value

  //synth_write_reg( 0x0c, 0x1f);   
  //synth_write_reg( 0x0d, 0xc0);   //allow the loop to lock

  /*
  printf("\r\nN1 = 0x%02x", (uint8_t) N1&0xff);
  printf("\r\nN2 = 0x%02x", (uint8_t) N2&0xff);

  printf("\r\nF1 = 0x%02x", (uint8_t) F1&0xff);
  printf("\r\nF2 = 0x%02x", (uint8_t) F2&0xff);
  printf("\r\nF3 = 0x%02x", (uint8_t) F3&0xff);

  printf("\r\nref = %f", REF);
  printf("\r\nfvco = %f", fvco);
  printf("\r\nO = %d", (int) odiv);
  printf("\r\nN = %d", (int) N);
  printf("\r\nF = %d", (int) F);
  */

  //wait_for_lock();
  //rf_unmute();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void rf_mute(void) {
  synth_write_reg( 0x02, 0x06); //mute RF output
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void rf_unmute(void) {
  synth_write_reg( 0x02, 0x04); //un-mute RF output
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void do_main_tasks(void)
{

  //extern void *pdev_device;

  //int offset;
  //int count;
  //int16_t len;

  if( VCP_get_string((unsigned char *) acm_in_buffer) ) {
    handle_command(ACM_INTF);
  } else {
    //len = eem_read_frame(Rx_Buff);

    //if( len > 0) {
     // LwIP_Pkt_Handle(Rx_Buff, len);
    //}
    //LwIP_Periodic_Handle(LocalTime);

  }

}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void Delay(__IO uint32_t nTime)
{
  TimingDelay = nTime;

  while(TimingDelay != 0);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void TimingDelay_Decrement(void)
{
  if (TimingDelay != 0x00) TimingDelay--;

  LocalTime+=75;
  if( LocalTime%300==0 && do_dump_constant) do_dump=1;

  do_freq_change=1;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
void delay_ms(int ms)
{
  DelayClk3( ms * 53333 );
}
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
uint8_t spi1_rx(void)
{
  int cnt=15000;
  while (cnt-->0 && SPI_I2S_GetFlagStatus(SPI1,  SPI_FLAG_RXNE) == RESET) {
    DelayClk3(1);
  }

  return (uint8_t)SPI_I2S_ReceiveData(SPI1);
}
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
uint8_t spi1_tx(uint8_t byte)
{
  int cnt=15000;
  while (cnt-->0 && SPI_I2S_GetFlagStatus(SPI1, SPI_FLAG_TXE) == RESET) {
    DelayClk3(1);
  }

  SPI_I2S_SendData(SPI1, byte);

  return spi1_rx();
}
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
uint8_t spi2_rx(void)
{
  int cnt=15000;
  while (cnt-->0 && SPI_I2S_GetFlagStatus(SPI2,  SPI_FLAG_RXNE) == RESET) {
    DelayClk3(1);
  }

  return (uint8_t)SPI_I2S_ReceiveData(SPI2);
}
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
uint8_t spi2_tx_slave(uint8_t byte)
{
  int cnt=15000;


  SPI_I2S_SendData(SPI2, byte);

  cnt=15000;
  while (cnt-->0 && SPI_I2S_GetFlagStatus(SPI2, SPI_FLAG_TXE) == RESET) {
    DelayClk3(1);
  }

  //while (cnt-->0 && SPI_I2S_GetFlagStatus(SPI2,  SPI_FLAG_RXNE) == RESET) {
  //  DelayClk3(1);
  //}


  return 0x00; 
}
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
void synth_select()
{
  GPIO_ResetBits(GPIOA, SYNTH_CS);
  DelayClk3(500);
}
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
void synth_deselect()
{
  DelayClk3(500);
  GPIO_SetBits(GPIOA, SYNTH_CS);
}


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
uint8_t synth_read_reg(uint8_t a)
{
  uint8_t addr = a;
  uint8_t val;

  synth_select();

  addr <<= 1;
  addr |= 0x01;  //read operation

  spi1_tx((uint8_t) addr);   //select register
  val = spi1_tx(0xff);

  synth_deselect();
  return val; 
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
void synth_write_reg(uint8_t a, uint8_t val)
{

  synth_select();

  uint8_t addr = a;

  addr <<= 1;
  addr &= 0xfe;  //write operation

  spi1_tx((uint8_t) addr);   //select register
  spi1_tx(val);    //write new val

  synth_deselect();

  //uint8_t r = synth_read_reg(a); 

  //printf("\r\nreg: %02x, wrote: %02x, read: %02x", a, val, r); 
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
void init_spi1(void)
{

  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);

  GPIO_StructInit(&GPIO_InitStruct);

  GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStruct.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7;
  GPIO_Init(GPIOA, &GPIO_InitStruct);

  GPIO_PinAFConfig(GPIOA, GPIO_PinSource5, GPIO_AF_SPI1);
  GPIO_PinAFConfig(GPIOA, GPIO_PinSource6, GPIO_AF_SPI1);
  GPIO_PinAFConfig(GPIOA, GPIO_PinSource7, GPIO_AF_SPI1);

  SPI_I2S_DeInit(SPI1);
  SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
  SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
  SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;
  SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;
  SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;

  SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_4; 

  SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
  SPI_InitStructure.SPI_CRCPolynomial = 7;
  SPI_InitStructure.SPI_Mode = SPI_Mode_Master;

  SPI_Init(SPI1, &SPI_InitStructure);

  SPI_Cmd(SPI1, ENABLE);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void init_spi2(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;
  NVIC_InitTypeDef NVIC_InitStructure;

  /* Peripheral Clock Enable -------------------------------------------------*/
  /* Enable the SPI clock */
  SPIx_CLK_INIT(SPIx_CLK, ENABLE);
  
  /* Enable GPIO clocks */
  RCC_AHB1PeriphClockCmd(SPIx_SCK_GPIO_CLK | SPIx_MISO_GPIO_CLK | SPIx_MOSI_GPIO_CLK, ENABLE);

  /* SPI GPIO Configuration --------------------------------------------------*/
  /* GPIO Deinitialisation */
  GPIO_DeInit(SPIx_SCK_GPIO_PORT);
  GPIO_DeInit(SPIx_MISO_GPIO_PORT);
  GPIO_DeInit(SPIx_MOSI_GPIO_PORT);
  
  /* Connect SPI pins to AF5 */  
  GPIO_PinAFConfig(SPIx_SCK_GPIO_PORT, SPIx_SCK_SOURCE, SPIx_SCK_AF);
  GPIO_PinAFConfig(SPIx_MISO_GPIO_PORT, SPIx_MISO_SOURCE, SPIx_MISO_AF);    
  GPIO_PinAFConfig(SPIx_MOSI_GPIO_PORT, SPIx_MOSI_SOURCE, SPIx_MOSI_AF);

  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_DOWN;

  /* SPI SCK pin configuration */
  GPIO_InitStructure.GPIO_Pin = SPIx_SCK_PIN;
  GPIO_Init(SPIx_SCK_GPIO_PORT, &GPIO_InitStructure);
  
  /* SPI  MISO pin configuration */
  GPIO_InitStructure.GPIO_Pin =  SPIx_MISO_PIN;
  GPIO_Init(SPIx_MISO_GPIO_PORT, &GPIO_InitStructure);  

  /* SPI  MOSI pin configuration */
  GPIO_InitStructure.GPIO_Pin =  SPIx_MOSI_PIN;
  GPIO_Init(SPIx_MOSI_GPIO_PORT, &GPIO_InitStructure);
 
  /* SPI configuration -------------------------------------------------------*/
  SPI_I2S_DeInit(SPIx);
  SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
  SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
  SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;
  SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;
  SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
  SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_4;
  SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
  SPI_InitStructure.SPI_CRCPolynomial = 7;
  
  /* Configure the Priority Group to 1 bit */                
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
  
  /* Configure the SPI interrupt priority */
  NVIC_InitStructure.NVIC_IRQChannel = SPIx_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);


  /* Slave board configuration */
  /* Initializes the SPI communication */
  SPI_InitStructure.SPI_Mode = SPI_Mode_Slave;
  SPI_Init(SPIx, &SPI_InitStructure);
  
  /* The Data transfer is performed in the SPI interrupt routine */
  /* Initialize Buffer counters */
  ubTxIndex = 0;
  ubRxIndex = 0;

  /* Enable the SPI peripheral */
  SPI_Cmd(SPIx, ENABLE);

  /* Enable the Rx buffer not empty interrupt */
  SPI_I2S_ITConfig(SPIx, SPI_I2S_IT_RXNE, ENABLE);
  
  /* Enable the Tx empty interrupt */
  //SPI_I2S_ITConfig(SPIx, SPI_I2S_IT_TXE, ENABLE);
  

}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void assert_param(int pass)
{
}
