
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
#include <float.h>
#include <complex.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include "std_io.h"
#include "fpga.h"
#include "crc32.h"
#include "telnet.h"
#include "globals.h"

#include "channel_filter.h"
#include "at86rf215.h"


#define INIT_CHANNEL_900 (20-1) //20 channels 
#define INIT_CHANNEL_24 (43-1)   //41 channels

static double ppm;
static double f1;
static double f2;
static double chan_step;
static double base_freq;
uint8_t tx_buffer[1514];

int is_fcc = 1;

static int fill_addr;
static uint16_t BBX_BASE;
static uint16_t FB_BASE;
static uint16_t RF_IRQS_BASE;
static uint16_t BBX_IRQS_BASE;
static uint16_t CMD_BASE;

int do_channel_scan;
int testing_tx_idx;

static int did_atrf_init24 = 0;
static int did_atrf_init900 = 0;
static int tx_wait;
static uint16_t addr;
static uint8_t val;
static int lc = 0;

static uint32_t channel_free_histo[20];


/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
void atrf_set_mode()
{

  delay_ms( 1 );
  at86rf215_write_single( RF24_CMD, CMD_RF_TRXOFF );
  at86rf215_write_single( RF09_CMD, CMD_RF_TRXOFF );
  delay_ms( 1 );

  if( do_iq_mode ) {
    set_iqifc1_mode( RF_MODE_RF );  //use IQ mode

    //uint8_t val = at86rf215_read_single(RF_IQIFC0); //enable loopback of the tx/rx
    //val |= 0x80;
    //at86rf215_write_single( RF_IQIFC0, val);

  } else {
    set_iqifc1_mode( RF_MODE_BBRF );  //use bandband units, no IQ mode
  }

  delay_ms( 1 );
  at86rf215_write_single( RF24_CMD, CMD_RF_TRXOFF );

  at86rf215_write_single( RF09_CMD, CMD_RF_TRXOFF );
  delay_ms( 1 );
  at86rf215_write_single( RF09_CMD, CMD_RF_RX );
  delay_ms( 1 );
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
uint8_t get_agc_gain( void )
{
  uint8_t agcs = at86rf215_read_single( RF09_AGCS );
  agcs &= 0x1f;
  return agcs;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
void lock_agc( void )
{
  uint8_t agcc = at86rf215_read_single( RF09_AGCC );
  agcc |= 0x02;
  at86rf215_write_single( RF09_AGCC, agcc );

}
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
void unlock_agc( void )
{
  uint8_t agcc = at86rf215_read_single( RF09_AGCC );
  agcc &= 0xfd;
  at86rf215_write_single( RF09_AGCC, agcc );
}

#define AT_DELAY 500

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
static void at86rf_select()
{
  DelayClk3( AT_DELAY );
  HAL_GPIO_WritePin( GPIOC, GPIO_PIN_7, GPIO_PIN_RESET ); //fpga cs,  at86 cs
  HAL_GPIO_WritePin( GPIOG, GPIO_PIN_1, GPIO_PIN_RESET );
  DelayClk3( AT_DELAY );
}
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
static void at86rf_deselect()
{
  DelayClk3( AT_DELAY );
  HAL_GPIO_WritePin( GPIOC, GPIO_PIN_7, GPIO_PIN_SET ); //fpga cs,  at86 cs
  HAL_GPIO_WritePin( GPIOG, GPIO_PIN_1, GPIO_PIN_SET );
  DelayClk3( AT_DELAY );
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
static uint8_t _xmit_at86rf_a( uint8_t out_reg )
{

  uint8_t in_reg;
  uint8_t tx = out_reg;

  HAL_StatusTypeDef status;


  DelayClk3( AT_DELAY );
  at86rf_select();
  DelayClk3( AT_DELAY );

  status = HAL_SPI_TransmitReceive_IT( &hspi3, &tx, &in_reg, 1 );
  DelayClk3( AT_DELAY );
  return in_reg;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
static uint8_t _xmit_at86rf_v( uint8_t out_reg )
{

  uint8_t in_reg;
  uint8_t tx = out_reg;

  HAL_StatusTypeDef status;

  DelayClk3( AT_DELAY );
  status = HAL_SPI_TransmitReceive_IT( &hspi3, &tx, &in_reg, 1 );
  DelayClk3( AT_DELAY );


  at86rf_deselect();
  return in_reg;
}
//////////////////////////////////////////////////////////////////////////
// no de-select for block access
//////////////////////////////////////////////////////////////////////////
static uint8_t _xmit_at86rf_vv( uint8_t out_reg )
{

  uint8_t in_reg;
  uint8_t tx = out_reg;

  HAL_StatusTypeDef status;

  DelayClk3( AT_DELAY );
  status = HAL_SPI_TransmitReceive_IT( &hspi3, &tx, &in_reg, 1 );
  DelayClk3( AT_DELAY );
  return in_reg;

}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
uint8_t at86rf215_read_single( uint16_t addr )
{

  _xmit_at86rf_a( ( addr >> 8 ) & 0x3f ); //read
  _xmit_at86rf_a( ( addr & 0xff ) );
  return _xmit_at86rf_v( 0xff );
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
uint8_t at86rf215_write_single( uint16_t addr, uint8_t val )
{

  _xmit_at86rf_a( 0x80 | ( ( addr >> 8 ) & 0x3f ) ); //write
  _xmit_at86rf_a( ( addr & 0xff ) );
  _xmit_at86rf_v( val );
  return 0;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
static uint8_t at86rf215_read_block( uint16_t addr, uint8_t *buffer, int len )
{
  int i;
  _xmit_at86rf_a( ( addr >> 8 ) & 0x3f ); //read
  _xmit_at86rf_a( ( addr & 0xff ) );

  for( i = 0; i < len; i++ ) {
    buffer[i] = _xmit_at86rf_vv( 0xff );
  }
  at86rf_deselect();
  return 0;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
static uint8_t at86rf215_write_block( uint16_t addr, uint8_t *buffer, int len )
{
  int i;

  _xmit_at86rf_a( 0x80 | ( ( addr >> 8 ) & 0x3f ) ); //write
  _xmit_at86rf_a( ( addr & 0xff ) );

  for( i = 0; i < len; i++ ) {
    _xmit_at86rf_vv( buffer[i] );
  }
  at86rf_deselect();
  return 0;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static uint8_t at86rf215_write_block_stx( uint16_t RF_IRQS_ADDR, uint16_t CMD_ADDR, uint16_t addr, uint8_t *buffer, int len, int stx_offset )
{

  int fill_cnt;

  fill_addr = addr;

  _xmit_at86rf_a( 0x80 | ( ( fill_addr >> 8 ) & 0x3f ) ); //write
  _xmit_at86rf_a( ( fill_addr & 0xff ) );

  for( fill_cnt = 0; fill_cnt < stx_offset; fill_cnt++ ) {
    _xmit_at86rf_vv( buffer[fill_cnt] );
  }
  at86rf_deselect();

  at86rf215_write_single( CMD_ADDR, CMD_RF_TRXOFF );
  delay_ms( 1 );
  //start transmit
  at86rf215_write_single( CMD_ADDR, CMD_RF_TXPREP );

  wait_for_tx_rx_ready( RF_IRQS_ADDR );

  at86rf215_write_single( CMD_ADDR, CMD_RF_TX );

  //keep filling
  fill_addr += stx_offset;
  _xmit_at86rf_a( 0x80 | ( ( fill_addr >> 8 ) & 0x3f ) ); //write
  _xmit_at86rf_a( ( fill_addr & 0xff ) );

  for( fill_cnt = 0; fill_cnt < len - stx_offset; fill_cnt++ ) {
    _xmit_at86rf_vv( buffer[fill_cnt + stx_offset] );
  }
  at86rf_deselect();

  return 0;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
int at86rf215_set_channel_900( int channel )
{
  if( channel > 19 ) return -1;

  at86rf215_write_single( RF09_CMD, CMD_RF_TRXOFF );  //trxoff -> txprep does calibration
  delay_ms( 1 );

  at86rf215_write_single( RF09_CNL, ( channel & 0xff ) ); //channel 0 ,  in this case, 0 = 903.2 MHz,  channel 19 = 926 MHz
  at86rf215_write_single( RF09_CNM, 0x00 ); //writing to CNM latches CNL,CS,CCF0L,CCF0H also  (must write to CNM to latch CNL)
  return 0;
}
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
int at86rf215_set_channel_24( int channel )
{
  if( channel > 42 ) return -1;

  at86rf215_write_single( RF24_CMD, CMD_RF_TRXOFF );  //trxoff -> txprep does calibration
  delay_ms( 1 );

  channel += 8; //start around 2412.0
  at86rf215_write_single( RF24_CNL, ( channel & 0xff ) ); //channel 0 ,
  at86rf215_write_single( RF24_CNM, ( channel >> 8 ) & 0x01 ); //writing to CNM latches CNL,CS,CCF0L,CCF0H also  (must write to CNM to latch CNL)

  return 0;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
void clear_cf_histo()
{
  int i;
  for( i = 0; i < 20; i++ ) {
    channel_free_histo[i] = 0;
  }
}


//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
int8_t get_rssi( void )
{
  int8_t rssi_dbm = at86rf215_read_single( RF09_RSSI ) + 12;
  return rssi_dbm;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
void _do_channel_scan()
{
  int i;

  atrf_check_init();

  do_iq_mode = 0;
  atrf_set_mode();  //switch back to non-iq mode


  //RX mode
  at86rf215_write_single( RF09_CMD, CMD_RF_RX );

  printf( "\r\n  " );
  for( i = 0; i < 20; i++ ) {

    at86rf215_set_channel_900( i );

    //RX
    at86rf215_write_single( RF09_CMD, CMD_RF_RX );
    delay_ms( 1 );

    int locked = ( at86rf215_read_single( RF09_PLL ) & 0x02 ) >> 1;

    int8_t rssi_dbm = at86rf215_read_single( RF09_RSSI );
    if( rssi_dbm < -113 ) rssi_dbm = -113; //noise floor for 1.2 Mhz channel

    printf( "\r\nchannel: %d (%3.1f MHz), rssi: %d dBm,  PLL lock-> %d, free_cnt: %5lu", i + 1, ( i * 1.2 ) + 903.2, rssi_dbm, locked, channel_free_histo[i] );

    if( rssi_dbm <= -111 )  {
      channel_free_histo[i]++;
      printf( "  <- clear channel" );
    }
  }



  delay_ms( 5 );
  do_iq_mode = 1;
  atrf_set_mode();  //switch back to iq mode
  delay_ms( 5 );

  at86rf215_write_single( RF09_CMD, CMD_RF_TXPREP );

  at86rf215_set_channel_900( INIT_CHANNEL_900 );

  at86rf215_write_single( RF09_CMD, CMD_RF_RX );
}


//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
void at86rf_read_id()
{

  atrf_check_init();

  //RX
  at86rf215_write_single( RF09_CMD, CMD_RF_RX );
  delay_ms( 1 );

  uint8_t ver = at86rf215_read_single( RF_PN );

  uint8_t rev = at86rf215_read_single( RF_VN );

  printf( "\r\npart id: AT86RF215: 0x%02x,  part rev: 0x%02x", ver, rev );


  int8_t rssi_dbm = at86rf215_read_single( RF09_RSSI );
  printf( "\r\nRSSI: %d dBm", rssi_dbm );

  at86rf215_write_single( RF09_CMD, CMD_RF_TRXOFF );  //trxoff -> txprep does calibration
  at86rf215_write_single( RF09_CMD, CMD_RF_TXPREP );
}

//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
uint8_t check_at86_bbx_irq( uint16_t BBX_IRQS_ADDR )
{

  uint8_t mask = 0;

  //status is cleared on read
  if( do_handle_exti15_10 ) {
    _xmit_at86rf_a( BBX_IRQS_ADDR >> 8 );
    _xmit_at86rf_a( ( BBX_IRQS_ADDR & 0xff ) );
    mask = _xmit_at86rf_v( 0xff );

    if( mask & IRQM_FBLI ) printf( "\r\nint source: FBLI" );
    if( mask & IRQM_AGCR ) printf( "\r\nint source: AGCR" );
    if( mask & IRQM_AGCH ) printf( "\r\nint source: AGCH" );
    if( mask & IRQM_TXFE ) printf( "\r\nint source: TXFE" );
    if( mask & IRQM_RXEM ) printf( "\r\nint source: RXEM" );
    if( mask & IRQM_RXAM ) printf( "\r\nint source: RXAM" );
    if( mask & IRQM_RXFE ) printf( "\r\nint source: RXFE" );
    if( mask & IRQM_RXFS ) printf( "\r\nint source: RXFS" );

    do_handle_exti15_10 = 0;
  }

  return mask;
}
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
uint8_t check_at86_rf_irq( uint16_t RF_IRQS_ADDR )
{

  uint8_t mask = 0;

  //status is cleared on read
  if( do_handle_exti15_10 ) {

    _xmit_at86rf_a( RF_IRQS_ADDR >> 8 );
    _xmit_at86rf_a( ( RF_IRQS_ADDR & 0xff ) );
    mask = _xmit_at86rf_v( 0xff );

    if( mask & IRQM_IQIFSF ) printf( "\r\nint source: IQIFSF" );
    if( mask & IRQM_TRXERR ) printf( "\r\nint source: TRXERR" );
    if( mask & IRQM_BATLOW ) printf( "\r\nint source: BATLOW" );
    if( mask & IRQM_EDC ) printf( "\r\nint source: EDC" );
    if( mask & IRQM_TRXRDY ) printf( "\r\nint source: TRXRDY" );
    if( mask & IRQM_WAKEUP ) printf( "\r\nint source: WAKEUP" );

    do_handle_exti15_10 = 0;
  }

  return mask;
}

//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
void wait_for_tx_rx_ready( uint16_t RF_IRQS_ADDR )
{
  tx_wait = 0;

  while( 1 ) {
    //if( (check_at86_rf_irq(RF_IRQS_ADDR) & IRQM_TRXRDY) || tx_wait++>800000 ) break;
    if( ( at86rf215_read_single( RF_IRQS_ADDR ) & IRQM_TRXRDY ) || tx_wait++ > 800000 ) break;
  }
}

//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
static void wait_for_tx_end( uint16_t BBX_IRQS_ADDR )
{
  tx_wait = 0;
  while( 1 ) {
    if( ( at86rf215_read_single( BBX_IRQS_ADDR ) & IRQM_TXFE ) || tx_wait++ > 800000 ) break;
  }
}

/////////////////////////////////////////////////////////////////////////////////////////////////
// RF_MODE_BBRF      RF enabled, baseband BBC0,BBC1 enabled, IQ mode disabled
// RF_MODE_RF        RF enabled, baseband BBC0,BBC1 disabled, IQ mode enabled
// RF_MODE_BBRF09    RF enabled, baseband BBC0 disabled, BBC1 enabled, IQ mode for 900 enabled
// RF_MODE_BBRF24    RF enabled, baseband BBC0 enabled,BBC1 disabled, IQ mode for 2.4 enabled
/////////////////////////////////////////////////////////////////////////////////////////////////
void set_iqifc1_mode( uint8_t mode )
{

  mode |= 0x03;  //default skew
  at86rf215_write_single( RF_IQIFC1, mode );

  at86rf215_write_single( RF_IQIFC0, 0x15 );  //enable embedded tx control EEC bit 1
  //at86rf215_write_single( RF_IQIFC0, 0x17);   //1.2V IEEE mode, enable embedded tx control EEC bit 1
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
void check_tx_sync()
{
  //uint8_t val1 = at86rf215_read_single( RF_IQIFC0 );
  uint8_t val2 = at86rf215_read_single( RF_IQIFC2 );
  //printf("\r\nTX IQ sync: %02x, %02x", val1, val2);
  if( ( val2 & 0x80 ) == 0x80 ) mcu_led_on();
  else mcu_led_off();
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
void atrf_init_24()
{
  at86rf215_write_single( RF24_PAC, 0x00 );     //disable tx power

  at86rf215_write_single( RF_CLKO, 0x08 );  //no CLKO

  at86rf215_write_single( RF24_CS, 0x30 );  //1.2 MHz channel spacing
  at86rf215_write_single( RF24_CCF0L, 0xf8 );
  at86rf215_write_single( RF24_CCF0H, 0x8c );

  at86rf215_set_channel_24( INIT_CHANNEL_24 ); //transitions to TRXOFF state, necessary to set frequency and power level

  at86rf215_write_single( RF24_RXBWC, 0x09 );   //FBW=1.25MHz,  Fif = 2MHz

  if( do_iq_mode ) {
    at86rf215_write_single( RF24_RXDFE, 0x02 );   //4mhz sample rate for iq mode
  } else {
    at86rf215_write_single( RF24_RXDFE, 0x03 );   //good for 1mhz ofdm
  }

  at86rf215_write_single( BBC1_IRQM, 0x00 );    //mask for interrupt pin
  at86rf215_write_single( RF24_IRQM, 0x00 );


  //write   BBCX_PC   bit 1-0  are Phy-type  00=0ff, 01=fsk, 10=ofdm, 11=O-DS
  at86rf215_write_single( BBC1_PC, 0x56 );

  at86rf215_write_single( BBC1_OFDMPHRTX, 0x05 );
  //    //OFDM MCS option 0-6  0 = BPSK 1/2 rate, 4x reps
  //   1 = QPSK 1/2 rate, 4x reps
  //   2 = QPSK 1/2 rate, 2x reps
  //   3 = QPSK 1/2 rate,
  //   4 = QPSK 3/4 rate,
  //   5 = 16-QAM 1/2 rate
  //   6 = 16-QAM 3/4 rate
  //

  at86rf215_write_single( BBC1_OFDMC, 0x00 );
  //OFDMC.OPT = 0   (option 1)

  at86rf215_write_single( RF24_EDD, 0x7a );

  at86rf215_write_single( RF24_TXCUTC, 0x0b );  //4uS PA ramp,   LPFCUT = 1 MHz

  at86rf215_write_single( RF24_TXDFE, 0x83 );   //tx RCUT and TX Sample Rate

  if( is_fcc ) {
    ///at86rf215_write_single( RF24_PAC, 0x77 );     //tx power control
  } else {
    ///at86rf215_write_single( RF24_PAC, 0x7c );     //0x7c is recommended max for OFDM, but less close-in distortion with 0x77
  }

  at86rf215_write_single( BBC1_TXFLL, 0xff );
  at86rf215_write_single( BBC1_TXFLH, 0x07 );  //len=2047


  //may want to experiment with this.  if it improves performance, then we can give up some freq change settle time
  if( is_fcc ) {
    at86rf215_write_single( RF24_PLL, 0x1a );  //PLL loop bandwidth descrease by 15% from default
  } else {
    at86rf215_write_single( RF24_PLL, 0x0a );  //PLL loop bandwidth default
  }

  at86rf215_write_single( RF24_CMD, CMD_RF_TRXOFF );  //trxoff -> txprep does calibration
  at86rf215_write_single( RF24_CMD, CMD_RF_TXPREP );
  delay_ms( 1 );

  at86rf215_read_single( RF24_IRQS );

  did_atrf_init24 = 1;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
double set_frequency_900mhz( double freq )
{

  uint8_t CM = 0x00;
  uint8_t CCF0H;
  uint8_t CCF0L;
  uint8_t CNL;

  ppm = -1.45;   //ppm proto 3,  hp sig gen is set to 915.00035 (using Siglent for cal)

  f1 = freq * ppm;
  f2 = f1 / 1000000.0;

  if( do_freq_offset ) freq = freq + f2;
  //if(do_low_if_mix) freq = freq - (double) 0.0125;   //low-IF offset before down-mix
  if( do_low_if_mix ) freq = freq - ( double ) 0.0125; //low-IF offset before down-mix

  at86rf215_write_single( RF09_CMD, CMD_RF_TRXOFF );  //trxoff -> txprep does calibration
  delay_us( 1 );

  int8_t freq_valid = 0;

  if( freq >= 389.5 && freq <= 510.0 ) {
    CM = ( 0x01 << 6 );
    chan_step = 99.182;
    base_freq = freq - 377.0;
    freq_valid = 1;
  } else if( freq >= 779.0 && freq <= 1020.0 ) {
    CM = ( 0x02 << 6 );
    chan_step = 198.364;
    base_freq = freq - 754.0;
    freq_valid = 1;
  }

  if( freq_valid ) {
    uint32_t CCF = ( uint32_t )( ( base_freq * 1e6 ) / chan_step );
    CCF0H = ( uint8_t )( CCF >> 16 ) & 0xff;
    CCF0L = ( uint8_t )( CCF >> 8 ) & 0xff;
    CNL = ( CCF & 0xff );

    //printf("\r\nCCF: %02x : %02x : %02x", CCF0H, CCF0L, CNL );

    at86rf215_write_single( RF09_CCF0L, CCF0L );
    at86rf215_write_single( RF09_CCF0H, CCF0H );
    at86rf215_write_single( RF09_CNL, CNL );

    at86rf215_write_single( RF09_CNM, CM );
    delay_us( 1 );
  }

  at86rf215_write_single( RF09_PLL, 0x09 );
  delay_us( 1 );
  at86rf215_write_single( RF09_PLL, 0x18 );

  return chan_step;

}
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
void at86rf215_set_gain( int gain )
{

  uint8_t AGCC = at86rf215_read_single( RF09_AGCC );

  if( gain == 0 ) {
    AGCC |= 0x01;   //enable AGC
    at86rf215_write_single( RF09_AGCC, AGCC );
    return;
  }

  AGCC = at86rf215_read_single( RF09_AGCC );
  AGCC &= 0xfe;
  at86rf215_write_single( RF09_AGCC, AGCC );  //disable AGC


  uint8_t AGCS = at86rf215_read_single( RF09_AGCS );
  AGCS &= 0xe0;
  AGCS |= ( gain & 0x1f );
  at86rf215_write_single( RF09_AGCS, AGCS );  //write gain control word

}
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
void at86rf215_set_bw( int bw )
{
  uint8_t RXBWC = at86rf215_read_single( RF09_RXBWC );
  //RXBWC &= 0xf0;
  RXBWC = 0x00; //defaults for upper 4
  RXBWC |= ( ( uint8_t ) bw );
  at86rf215_write_single( RF09_RXBWC, RXBWC );
}
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
void at86rf215_set_bw_ifi( int bw )
{
  uint8_t RXBWC = at86rf215_read_single( RF09_RXBWC );
  RXBWC &= 0xf0;
  RXBWC |= 0x20;
  RXBWC |= ( ( uint8_t ) bw );
  at86rf215_write_single( RF09_RXBWC, RXBWC );
}
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
void at86rf215_set_bw_ifs( int bw )
{
  uint8_t RXBWC = at86rf215_read_single( RF09_RXBWC );
  RXBWC &= 0xf0;
  RXBWC |= 0x10;
  RXBWC |= ( ( uint8_t ) bw );
  at86rf215_write_single( RF09_RXBWC, RXBWC );
}
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
void at86rf215_set_bw_ifi_ifs( int bw )
{
  uint8_t RXBWC = at86rf215_read_single( RF09_RXBWC );
  RXBWC &= 0xf0;
  RXBWC |= 0x20;
  RXBWC |= 0x10;
  RXBWC |= ( ( uint8_t ) bw );
  at86rf215_write_single( RF09_RXBWC, RXBWC );
}
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
uint8_t at86rf215_set_rcut( int rcut )
{
  uint8_t RXDFE = at86rf215_read_single( RF09_RXDFE );
  RXDFE &= 0x1f;
  RXDFE |= ( ( uint8_t ) rcut ) << 5;
  at86rf215_write_single( RF09_RXDFE, RXDFE );
  return at86rf215_read_single( RF09_RXDFE );
}
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
void at86rf215_set_srate( int rate )
{
  uint8_t RXDFE = at86rf215_read_single( RF09_RXDFE );
  RXDFE &= 0xe0;

  switch( rate ) {
  case  400000  :
    at86rf215_write_single( RF09_RXDFE, ( RXDFE | 0x0a ) ); //400 KHz
    break;

  case  500000  :
    at86rf215_write_single( RF09_RXDFE, ( RXDFE | 0x08 ) ); //500 KHz
    break;

  case  666666  :
    at86rf215_write_single( RF09_RXDFE, ( RXDFE | 0x06 ) ); //666666Hz
    break;

  case  800000 :
    at86rf215_write_single( RF09_RXDFE, ( RXDFE | 0x05 ) ); //800 KHz
    break;

  case  1000000 :
    at86rf215_write_single( RF09_RXDFE, ( RXDFE | 0x04 ) ); //1000 KHz
    break;

  case  1333333 :
    at86rf215_write_single( RF09_RXDFE, ( RXDFE | 0x03 ) ); //1333KHz
    break;

  case  2000000  :
    at86rf215_write_single( RF09_RXDFE, ( RXDFE | 0x02 ) ); //2000 KHz
    break;

  case  4000000  :
    at86rf215_write_single( RF09_RXDFE, ( RXDFE | 0x01 ) ); //4000 KHz
    break;

  default :
    at86rf215_write_single( RF09_RXDFE, ( RXDFE | 0x05 ) ); //800 KHz
    break;
  }
};

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
void atrf_init_900()
{
  at86rf215_write_single( RF09_PAC, 0x00 );     //disable TX power

  //at86rf215_write_single( RF09_AGCS, 0x77 );  //target AGC = -30dB  //default
  at86rf215_write_single( RF09_AGCS, 0xf7 );  //target AGC = -42dB, this increases max input level and keeps 8-bit I/Q outputs in range

  //at86rf215_write_single( RF09_AGCC, 0xb1 );  //enables 64 samples per AGC update vs 8

  at86rf215_write_single( RF_CLKO, 0x08 );  //no CLKO

  at86rf215_write_single( RF09_CS, 0x30 );  //1.2 MHz channel spacing

  //at86rf215_write_single( RF09_CCF0L, 0x20 ); //set frequency  (903.2e6 / 25khz)=0x8d20,  note: 25khz resolution is derived from
  //RF09_CS (0x30)->  1.2e6/0x30 = 25khz
  //at86rf215_write_single( RF09_CCF0H, 0x8d );

  //at86rf215_set_channel_900(INIT_CHANNEL_900);  //transitions to TRXOFF state, necessary to set frequency and power level

  //at86rf215_set_channel_900(0);

  set_frequency_900mhz( 915.0 );


  if( do_iq_mode ) {
    //OFDM 800khz channel
    //at86rf215_write_single( RF09_RXBWC, 0x19 ); //bit 4 is IF inversion
    //at86rf215_write_single( RF09_RXDFE, 0x83 );

    //at86rf215_write_single( RF09_RXBWC, 0x10 );     //cut = 160khz , shift IF by 1.25 x,  250e3 x 1.25
    at86rf215_write_single( RF09_RXBWC, 0x00 );     //cut = 160khz
    //at86rf215_write_single( RF09_RXBWC, 0x0b );     //cut = 2000 kHz

    //at86rf215_write_single( RF09_RXBWC, 0x04 );     //cut = 400khz
    //at86rf215_write_single( RF09_RXBWC, 0x06 );     //cut = 630khz
    //at86rf215_write_single( RF09_RXBWC, 0x07 );     //cut = 800khz

    //at86rf215_write_single( RF09_RXDFE, 0x02 );   //2000 KHz  works
    at86rf215_write_single( RF09_RXDFE, 0x05 );   //800 KHz  //msb = 1, |0x80  is RCUT=FS/2,  0= RCUT, FS/8
    //at86rf215_write_single( RF09_RXDFE, 0x06 );   //666 KHz
    //at86rf215_write_single( RF09_RXDFE, 0x0a );   //400 KHz
  } else {
    at86rf215_write_single( RF09_RXBWC, 0x09 );   //FBW=1.25MHz,  Fif = 2MHz
    at86rf215_write_single( RF09_RXDFE, 0x83 );   //good for 1mhz ofdm
  }

  at86rf215_write_single( BBC0_IRQM, 0x00 );  //mask for interrupt pin output
  at86rf215_write_single( RF09_IRQM, 0x00 );


  //write   BBC0_PC   bit 1-0  are Phy-type  00=0ff, 01=fsk, 10=ofdm, 11=O-DS
  at86rf215_write_single( BBC0_PC, 0x56 );

  at86rf215_write_single( BBC0_OFDMPHRTX, 0x03 );
  //    //OFDM MCS option 0-6  0 = BPSK 1/2 rate, 4x reps
  //   1 = QPSK 1/2 rate, 4x reps
  //   2 = QPSK 1/2 rate, 2x reps
  //   3 = QPSK 1/2 rate,
  //   4 = QPSK 3/4 rate,
  //   5 = 16-QAM 1/2 rate
  //   6 = 16-QAM 3/4 rate
  //

  at86rf215_write_single( BBC0_OFDMC, 0x00 );
  //OFDMC.OPT = 0   (option 1)

  at86rf215_write_single( RF09_EDD, 0x7a );

  at86rf215_write_single( RF09_TXCUTC, 0x0b );  //4uS PA ramp,   LPFCUT = 1 MHz

  at86rf215_write_single( RF09_TXDFE, 0x83 );   //tx RCUT and TX Sample Rate

  if( is_fcc ) {
    //at86rf215_write_single( RF09_PAC, 0x7f );     //tx power control
    //0x77 may be best choice with baseband for linearity.
    //with firmware as of 2018-03-22 12:09,  0x7c gives about
    //1 dB increase in 8dBm PSD margin allowing for 32.2dBm peak power CW.
    //channel power will be approximately 26.2 dBm on the analyzer  (-6dB for modulation)
    //final amplifier + loss + gain will need to be ~28dB. may want a pre-driver w/saw
  } else {
    //at86rf215_write_single( RF09_PAC, 0x7c );     //0x7c is recommended max for OFDM, but less close-in distortion with 0x77
  }
  at86rf215_write_single( RF09_PAC, 0x00 );     //disable TX power

  at86rf215_write_single( RF09_AUXS, 0x02 );     //voltage on the final PA  0x00=2V,  0x02 = 2.4V (default)
  //at86rf215_write_single( RF09_AUXS, 0x22 );     //voltage on the final PA  0x00=2V,  0x02 = 2.4V (default), external LNA gain=9
  //at86rf215_write_single( RF09_AUXS, 0x42 );     //voltage on the final PA  0x00=2V,  0x02 = 2.4V (default), external LNA gain=12



  at86rf215_write_single( RF09_PADFE, 0x00 );     //no external PA/LNA control

  at86rf215_write_single( BBC0_TXFLL, 0xff );
  at86rf215_write_single( BBC0_TXFLH, 0x07 );  //len=2047


  //may want to experiment with this.  if it improves performance, then we can give up some freq change settle time
  if( is_fcc ) {
    at86rf215_write_single( RF09_PLL, 0x1a );  //PLL loop bandwidth descrease by 15% from default
  } else {
    at86rf215_write_single( RF09_PLL, 0x0a );  //PLL loop bandwidth default
  }

  at86rf215_write_single( RF09_CMD, CMD_RF_TRXOFF );  //trxoff -> txprep does calibration
  at86rf215_write_single( RF09_CMD, CMD_RF_TXPREP );
  delay_ms( 1 );

  at86rf215_read_single( RF09_IRQS );
  did_atrf_init900 = 1;

}

/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
void transmit_frame( uint16_t RF_BASE, uint8_t *buffer, int len )
{

  atrf_check_init();

  if( RF_BASE == RF24_BASE ) {
    //24 base
    BBX_BASE = BBC1_BASE;
    FB_BASE = FB1_BASE;
    BBX_IRQS_BASE = BBC1_IRQS;
    RF_IRQS_BASE = RF24_IRQS;
    CMD_BASE = RF24_CMD;

  } else {
    //900 base
    BBX_BASE = BBC0_BASE;
    FB_BASE = FB0_BASE;
    RF_IRQS_BASE = RF09_IRQS;
    BBX_IRQS_BASE = BBC0_IRQS;
    CMD_BASE = RF09_CMD;
  }

  len += 4; //add length for FCS bytes  (32-bit crc)
  at86rf215_write_single( ( BBX_BASE + BBCX_TXFLL ), ( len & 0xff ) );
  at86rf215_write_single( ( BBX_BASE + BBCX_TXFLH ), ( len >> 8 ) & 0x07 ); //set length
  len -= 4;

  //crc32_val = 0;
  //uint32_t bcrc = crc32_range( buffer, len-4 );
  //memcpy(&buffer[len-4], &bcrc, 4);
  //delay_ms(1);

  if( len > 64 ) {
    //write to tx fifo
    at86rf215_write_block_stx( RF_IRQS_BASE, CMD_BASE, ( FB_BASE + BBCX_FBTXS ), buffer, len, 16 ); //has to be at least 256 on stm32f4xx@168MHz With SPI prescaler=2
  } else {
    at86rf215_write_block( ( FB_BASE + BBCX_FBTXS ), buffer, len );
    //transmit it
    at86rf215_write_single( ( RF_BASE + RFXX_CMD ), CMD_RF_TX );
  }

  wait_for_tx_end( BBX_IRQS_BASE );

  //bit0 is under-run flag
  //printf("\r\nBBC0_PS: %02x", at86rf215_read_single(BBC0_PS) );
}

/////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////
void do_tx_test( uint16_t RF_BASE )
{

  atrf_check_init();

  if( RF_BASE == RF24_BASE ) {
    //24 base
    BBX_BASE = BBC1_BASE;
    FB_BASE = FB1_BASE;
    BBX_IRQS_BASE = BBC1_IRQS;
  } else {
    //900 base
    BBX_BASE = BBC0_BASE;
    FB_BASE = FB0_BASE;
    BBX_IRQS_BASE = BBC0_IRQS;
  }


  if( do_rf_tx_test ) {

    transmit_frame( RF_BASE, tx_buffer, 1514 );


    if( testing_tx_idx == 10000 ) {
      //do_rf_tx_test=0;
    }

    if( testing_tx_idx % 50 == 0 ) printf( "\r\ntx count: %d,  tx_bytes: %d", testing_tx_idx, testing_tx_idx * 1500 );
    testing_tx_idx++;

    if( !do_rf_tx_test ) print_prompt();
  }

}


////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////
void atrf_dump_regs()
{


  atrf_check_init();

  do_iq_mode = 1;
  atrf_set_mode();


  addr = 0x0000;
  while( 1 ) {
    val = at86rf215_read_single( addr );
    printf( "\r\n%04x: 0x%02x:  %d%d%d%d %d%d%d%d", addr, val,
            ( val & 0x80 ) >> 7,
            ( val & 0x40 ) >> 6,
            ( val & 0x20 ) >> 5,
            ( val & 0x10 ) >> 4,
            ( val & 0x08 ) >> 3,
            ( val & 0x04 ) >> 2,
            ( val & 0x02 ) >> 1,
            ( val & 0x01 ) );

    addr++;
    if( addr == 0x00f ) break;
    if( lc++ % 16 == 0 ) while( net_buffer[0] != 0 ) main_tick();
  }

  addr = 0x0100;
  while( 1 ) {
    val = at86rf215_read_single( addr );
    printf( "\r\n%04x: 0x%02x:  %d%d%d%d %d%d%d%d", addr, val,
            ( val & 0x80 ) >> 7,
            ( val & 0x40 ) >> 6,
            ( val & 0x20 ) >> 5,
            ( val & 0x10 ) >> 4,
            ( val & 0x08 ) >> 3,
            ( val & 0x04 ) >> 2,
            ( val & 0x02 ) >> 1,
            ( val & 0x01 ) );

    addr++;
    if( addr == 0x129 ) break;
    if( lc++ % 16 == 0 ) while( net_buffer[0] != 0 ) main_tick();
  }


  addr = 0x0200;
  while( 1 ) {
    val = at86rf215_read_single( addr );
    printf( "\r\n%04x: 0x%02x:  %d%d%d%d %d%d%d%d", addr, val,
            ( val & 0x80 ) >> 7,
            ( val & 0x40 ) >> 6,
            ( val & 0x20 ) >> 5,
            ( val & 0x10 ) >> 4,
            ( val & 0x08 ) >> 3,
            ( val & 0x04 ) >> 2,
            ( val & 0x02 ) >> 1,
            ( val & 0x01 ) );

    addr++;
    if( addr == 0x229 ) break;
    if( lc++ % 16 == 0 ) while( net_buffer[0] != 0 ) main_tick();
  }



  addr = 0x0300;
  while( 1 ) {
    val = at86rf215_read_single( addr );
    printf( "\r\n%04x: 0x%02x:  %d%d%d%d %d%d%d%d", addr, val,
            ( val & 0x80 ) >> 7,
            ( val & 0x40 ) >> 6,
            ( val & 0x20 ) >> 5,
            ( val & 0x10 ) >> 4,
            ( val & 0x08 ) >> 3,
            ( val & 0x04 ) >> 2,
            ( val & 0x02 ) >> 1,
            ( val & 0x01 ) );

    addr++;
    if( addr == 0x395 ) break;
    if( lc++ % 16 == 0 ) while( net_buffer[0] != 0 ) main_tick();
  }

  addr = 0x0400;
  while( 1 ) {
    val = at86rf215_read_single( addr );
    printf( "\r\n%04x: 0x%02x:  %d%d%d%d %d%d%d%d", addr, val,
            ( val & 0x80 ) >> 7,
            ( val & 0x40 ) >> 6,
            ( val & 0x20 ) >> 5,
            ( val & 0x10 ) >> 4,
            ( val & 0x08 ) >> 3,
            ( val & 0x04 ) >> 2,
            ( val & 0x02 ) >> 1,
            ( val & 0x01 ) );

    addr++;
    if( addr == 0x495 ) break;
    if( lc++ % 16 == 0 ) while( net_buffer[0] != 0 ) main_tick();
  }

  print_prompt();

}

///////////////////////////////////////////////////////////////
// configure GPIOF PIN 0 as audio interrupt for fifo fill
// was ALT function of A0 for address bus.  not used as A0
// since bus is multiplexed
///////////////////////////////////////////////////////////////
void rf900_int_config( void )
{
  GPIO_InitTypeDef   GPIO_InitStructure;

  // Enable GPIOF clock
  __HAL_RCC_GPIOF_CLK_ENABLE();

  GPIO_InitStructure.Mode = GPIO_MODE_IT_RISING_FALLING;
  GPIO_InitStructure.Pull = GPIO_NOPULL;
  GPIO_InitStructure.Pin = GPIO_PIN_1;
  HAL_GPIO_Init( GPIOF, &GPIO_InitStructure );

  // Enable and set EXTI lines 1 Interrupt priority
  HAL_NVIC_SetPriority( EXTI1_IRQn, 0, 0 ); //iq fifo rx priority
  HAL_NVIC_EnableIRQ( EXTI1_IRQn );
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
void atrf_check_init()
{

  if( !did_atrf_init24 && !did_atrf_init900 ) {

    delay_ms( 50 );
    //RF Reset
    at86rf215_write_single( RF09_CMD, CMD_RF_RESET );
    delay_ms( 50 );

  }
  if( !did_atrf_init24 ) {
    atrf_init_24();
  }
  if( !did_atrf_init900 ) {
    atrf_init_900();
  }
}
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
void udp_send_response_iq900( uint8_t *data, int n )
{

#if 1
  volatile struct pbuf *p;

  p = pbuf_alloc( PBUF_TRANSPORT, n, PBUF_RAM );
  if( p != NULL ) {

    memcpy( p->payload, ( uint8_t * ) data, n );
    udp_sendto( udp_data_pcb, p, &udp_saddr, 8889 );
    pbuf_free( p );
  }
#endif

}
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
void udp_send_response_demod( uint8_t *buffer, int len )
{

#if 1
  volatile struct pbuf *p;
  char b[1024];
  int l = len;

  if( l > 1024 ) l = 1024;
  memcpy( b, buffer, l );

  p = pbuf_alloc( PBUF_TRANSPORT, l, PBUF_RAM );
  if( p != NULL ) {

    memcpy( p->payload, ( uint8_t * ) b, l );
    udp_sendto( udp_data_pcb, p, &udp_saddr, 9888 );
    pbuf_free( p );
  }
#endif

}
