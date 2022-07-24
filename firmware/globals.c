
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



#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include <float.h>
#include <complex.h>

#include "flash_s08.h"
#include "globals.h"
#include "crc32.h"

extern void MX_SPI3_Init( void ); //must be after fpga bit-bang init

int do_test_mem;
int do_test_flash;
int do_write_config;
int do_read_config;
int do_system_reset;
int do_iq_mode;
int do_rf_tx_test;
int do_at_dump;
int do_at_scan;
int do_at_scan_cont;

static config_t configuration;
config_t *_config;
config_t1 *config;

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
int read_configuration_from_flash( void )
{
  flash_read( CONFIG_ADDRESS, ( uint8_t * ) &configuration, CONFIG_SIZE, 0 );
  crc32_val = 0x0000;
  uint32_t crc = crc32_range( ( uint8_t * ) &configuration, CONFIG_SIZE - 4 );

  if( crc != _config->crc32 ) return 1;
  return 0;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
void write_configuration_to_flash( void )
{
  crc32_val = 0x0000;
  uint32_t crc = crc32_range( ( uint8_t * ) &configuration, CONFIG_SIZE - 4 );
  _config->crc32 = crc;

  flash_write_blocking( CONFIG_ADDRESS, ( uint8_t * ) &configuration, CONFIG_SIZE, 0 );
  return;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
static void reset_config_to_defaults( void )
{
  do_test_mem = 0;
  do_test_flash = 0;
  do_iq_mode = 0;
  do_rf_tx_test = 0;
  do_read_config = 0;
  do_write_config = 0;
  do_system_reset = 0;
  do_at_scan = 0;
  do_at_scan_cont = 0;

  config->audio_on = 1;
  config->audio_volume_f = 0.75f;
  config->p25_logging = 1; //1=print headers, 2+ == more details

  config->p25_radiomanf_id = DEFAULT_P25_RADIO_MANF;
  config->p25_radiomanf_id_allow_all = 1;
  config->p25_network_id = DEFAULT_P25_NETWORK;
  config->p25_network_id_allow_all = 1;

  config->uart3_baudrate = 2000000;

  config->ip_addr[0] = 192;
  config->ip_addr[1] = 168;
  config->ip_addr[2] = 1;
  config->ip_addr[3] = 150;

  config->scanner_mode = 0;
  config->squelch = -126;
  config->srate = 800000;
  config->rcut = 0;
  config->bw = 0;
  config->rfgain = 0; //0=auto
  config->i_off = 0.0;
  config->q_off = 0.0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
// call in main.c during init
// init variables that can be changed via the command interface /scripts / non-volatile config, etc
////////////////////////////////////////////////////////////////////////////////////////////////////////
void init_globals( void )
{

  _config = &configuration;
  _config->config = &( _config->configt1 );
  config = _config->config;

  MX_SPI3_Init();  //must be after fpga bit-bang init
  init_flash_s08();

  //int err = read_configuration_from_flash();
  int err = 1;

  if( err ) {
    reset_config_to_defaults();
    //write_configuration_to_flash();
  }
}

/////////////////////////////////////////////
// delay for 3 clock cycles * ulCount
/////////////////////////////////////////////
void __attribute__( ( naked ) ) DelayClk3( unsigned long ulCount )
{
  __asm( "    subs    r0, #1\n"
         "    bne     DelayClk3\n"
         "    bx      lr" );
}

/////////////////////////////////////////////
/////////////////////////////////////////////
void delay_ms( int delay )
{
  HAL_Delay( delay );
}
/////////////////////////////////////////////
/////////////////////////////////////////////
void delay_us( int delay )
{
  DelayClk3( delay * 134 );
}
