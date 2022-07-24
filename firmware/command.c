
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




#include <string.h>
#include <math.h>
#include <float.h>
#include <complex.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include "globals.h"
#include "command.h"
#include "telnet.h"
#include "std_io.h"
#include "telnet.h"
#include "at86rf215.h"

static int reset_timer;
static int at_scan_timer;
uint8_t _xmit_tuner( uint8_t out_reg );

//////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////
void parse_command( char *cmd )
{

  char cmdstr[CMD_BUFFER_LEN];
  strncpy( cmdstr, cmd, CMD_BUFFER_LEN );


  cmdstr[CMD_BUFFER_LEN - 1] = 0x00;

  char *ptr = cmdstr;

  //trim off end of cmd string
  ptr += strlen( ( char * ) cmdstr ) - 5;
  if( *ptr == '\r' || *ptr == '\n' ) *ptr = 0x00;
  ptr++;
  if( *ptr == '\r' || *ptr == '\n' ) *ptr = 0x00;
  ptr++;
  if( *ptr == '\r' || *ptr == '\n' ) *ptr = 0x00;
  ptr++;
  if( *ptr == '\r' || *ptr == '\n' ) *ptr = 0x00;


  const char s[2] = " "; //separator
  char *token;

  //eight arguments should be enough for anybody
  char *tok1 = NULL;
  char *tok2 = NULL;
  char *tok3 = NULL;
  char *tok4 = NULL;
  char *tok5 = NULL;
  char *tok6 = NULL;
  char *tok7 = NULL;
  char *tok8 = NULL;

  int toks = 0;

  token = strtok( cmdstr, s );
  while( token != NULL && toks < 8 ) {
    switch( ++toks ) {
    case 1 :
      tok1 = token;
      break;
    case 2 :
      tok2 = token;
      break;
    case 3 :
      tok3 = token;
      break;
    case 4 :
      tok4 = token;
      break;
    case 5 :
      tok5 = token;
      break;
    case 6 :
      tok6 = token;
      break;
    case 7 :
      tok7 = token;
      break;
    case 8 :
      tok8 = token;
      break;
    }
    token = strtok( NULL, s );
  }

#if 0
  if( tok1 != NULL ) printf( "\r\ntok1: %s", tok1 );
  if( tok2 != NULL ) printf( "\r\ntok2: %s", tok2 );
  if( tok3 != NULL ) printf( "\r\ntok3: %s", tok3 );
  if( tok4 != NULL ) printf( "\r\ntok4: %s", tok4 );
  if( tok5 != NULL ) printf( "\r\ntok5: %s", tok5 );
  if( tok6 != NULL ) printf( "\r\ntok6: %s", tok6 );
  if( tok7 != NULL ) printf( "\r\ntok7: %s", tok7 );
  if( tok8 != NULL ) printf( "\r\ntok8: %s", tok8 );
#endif

  int do_prompt = 1;
  int val1;

  ///////////////////////////////////////////////////////////////
  // TESTMEM
  ///////////////////////////////////////////////////////////////
  if( strcmp( tok1, ( const char * ) "testmem" ) == 0 ) {
    do_test_mem = 1;
  }
  ///////////////////////////////////////////////////////////////
  // AUDIO
  ///////////////////////////////////////////////////////////////
  else if( strcmp( tok1, ( const char * ) "audio" ) == 0 ) {


    val1 = command_is_one( tok2 ); //on/off 0/1
    if( val1 >= 0 && toks == 2 ) {
      config->audio_on = val1;
      printf( "\r\naudio %d", config->audio_on );
    } else if( strncmp( tok2, "vol", 3 ) == 0 ) {

      if( toks == 3 ) {
        config->audio_volume_f = atof( tok3 ); //audio vol xxx.xf, 0.0 to 1.0

        if( config->audio_volume_f < 0.0f ) config->audio_volume_f = 0.0f;
        else if( config->audio_volume_f > 1.0f ) config->audio_volume_f = 1.0f;
        else if( isnan( config->audio_volume_f ) ) config->audio_volume_f = 0.75f;
      }

      printf( "\r\naudio vol %3.2f", config->audio_volume_f );
    } else if( toks == 1 ) {
      printf( "\r\naudio %d, audio vol %3.2f", config->audio_on, config->audio_volume_f );
    }
  }
  ///////////////////////////////////////////////////////////////
  // IOFF
  ///////////////////////////////////////////////////////////////
  else if( strcmp( tok1, ( const char * ) "ioff" ) == 0 ) {


    if( toks == 2 ) {
      float _ioff = ( float ) atof( tok2 );
      config->i_off = _ioff;
    }
    printf( "\r\nioff %f", config->i_off );

  }
  ///////////////////////////////////////////////////////////////
  // QOFF
  ///////////////////////////////////////////////////////////////
  else if( strcmp( tok1, ( const char * ) "qoff" ) == 0 ) {


    if( toks == 2 ) {
      float _qoff = ( float ) atof( tok2 );
      config->q_off = _qoff;
    }
    printf( "\r\nqoff %f", config->q_off );

  }
  ///////////////////////////////////////////////////////////////
  // RFGAIN
  ///////////////////////////////////////////////////////////////
  else if( strcmp( tok1, ( const char * ) "rfgain" ) == 0 ) {


    if( toks == 2 ) {
      int _gain = ( int ) atoi( tok2 );
      if( _gain >= 0 && _gain <= 23 ) {
        config->rfgain = _gain;
        at86rf215_set_gain( config->rfgain );
      }
    }
    printf( "\r\nrfgain %d", config->rfgain );

  }
  ///////////////////////////////////////////////////////////////
  // BW
  ///////////////////////////////////////////////////////////////
  else if( strcmp( tok1, ( const char * ) "bw" ) == 0 ) {


    if( toks == 2 ) {
      int _bw = ( int ) atoi( tok2 );
      if( _bw >= 0 && _bw <= 11 ) {
        config->bw = _bw;
        at86rf215_set_bw( config->bw );
      }
    }
    printf( "\r\nbw %d", config->bw );

  }
  ///////////////////////////////////////////////////////////////
  // BWIFS
  ///////////////////////////////////////////////////////////////
  else if( strcmp( tok1, ( const char * ) "bwifs" ) == 0 ) {


    if( toks == 2 ) {
      int _bw = ( int ) atoi( tok2 );
      if( _bw >= 0 && _bw <= 11 ) {
        config->bw = _bw;
        at86rf215_set_bw_ifs( config->bw );
      }
    }
    printf( "\r\nbw %d", config->bw );

  }
  ///////////////////////////////////////////////////////////////
  // BWIFI
  ///////////////////////////////////////////////////////////////
  else if( strcmp( tok1, ( const char * ) "bwifi" ) == 0 ) {


    if( toks == 2 ) {
      int _bw = ( int ) atoi( tok2 );
      if( _bw >= 0 && _bw <= 11 ) {
        config->bw = _bw;
        at86rf215_set_bw_ifi( config->bw );
      }
    }
    printf( "\r\nbw %d", config->bw );

  }
  ///////////////////////////////////////////////////////////////
  // BWIFI
  ///////////////////////////////////////////////////////////////
  else if( strcmp( tok1, ( const char * ) "bwifiifs" ) == 0 ) {


    if( toks == 2 ) {
      int _bw = ( int ) atoi( tok2 );
      if( _bw >= 0 && _bw <= 11 ) {
        config->bw = _bw;
        at86rf215_set_bw_ifi_ifs( config->bw );
      }
    }
    printf( "\r\nbw %d", config->bw );

  }
  ///////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////
  // BWIFI
  ///////////////////////////////////////////////////////////////
  else if( strcmp( tok1, ( const char * ) "bwifi" ) == 0 ) {


    if( toks == 2 ) {
      int _bw = ( int ) atoi( tok2 );
      if( _bw >= 0 && _bw <= 11 ) {
        config->bw = _bw;
        at86rf215_set_bw_ifi( config->bw );
      }
    }
    printf( "\r\nbw %d", config->bw );

  }
  // RCUT
  ///////////////////////////////////////////////////////////////
  else if( strcmp( tok1, ( const char * ) "rcut" ) == 0 ) {

    uint8_t r = 0;
    if( toks == 2 ) {
      int _cut = ( int ) atoi( tok2 );
      if( _cut >= 0 && _cut <= 4 ) {
        config->rcut = _cut;
        r = at86rf215_set_rcut( config->rcut );
      }
    }
    printf( "\r\nrcut %02x", r );

  }
  ///////////////////////////////////////////////////////////////
  // SRATE
  ///////////////////////////////////////////////////////////////
  else if( strcmp( tok1, ( const char * ) "srate" ) == 0 ) {

    int do_srate = 0;

    if( toks == 2 ) {
      int _rate = ( int ) atoi( tok2 );
      switch( _rate ) {
      case   4000000  :
        config->srate = 4000000;
        do_srate = 1;
        break;
      case   2000000  :
        config->srate = 2000000;
        do_srate = 1;
        break;
      case   1333333  :
        config->srate = 1333333;
        do_srate = 1;
        break;
      case   1000000  :
        config->srate = 1000000;
        do_srate = 1;
        break;
      case   800000  :
        config->srate = 800000;
        do_srate = 1;
        break;
      case   666666  :
        config->srate = 666666;
        do_srate = 1;
        break;
      case   500000  :
        config->srate = 500000;
        do_srate = 1;
        break;
      case   400000  :
        config->srate = 400000;
        do_srate = 1;
        break;

      default :
        do_srate = 0;
        break;
      }
    }
    if( do_srate ) at86rf215_set_srate( config->srate );
    init_dc_correction();
    printf( "\r\nsrate %d", config->srate );

  }
  ///////////////////////////////////////////////////////////////
  // SQUELCH
  ///////////////////////////////////////////////////////////////
  else if( strcmp( tok1, ( const char * ) "squelch" ) == 0 ) {

    val1 = command_is_one( tok2 ); //on/off 0/1
    if( val1 >= 0 && toks == 2 ) {
      config->squelch = val1;
    } else if( toks == 2 ) {
      config->squelch = ( int ) atoi( tok2 ); //more detailed logging
    }
    printf( "\r\nsquelch %d", config->squelch );

  }
  ///////////////////////////////////////////////////////////////
  // SA MODE
  ///////////////////////////////////////////////////////////////
  else if( strcmp( tok1, ( const char * ) "sa" ) == 0 ) {

    val1 = command_is_one( tok2 ); //on/off 0/1
    if( val1 >= 0 && toks == 2 ) {
      config->sa_mode = val1;
    } else if( toks == 2 ) {
      config->sa_mode = ( int ) atoi( tok2 ); //more detailed logging
    }
    printf( "\r\nsa_mode %d", config->sa_mode );

  }
  ///////////////////////////////////////////////////////////////
  // SCANNER MODE
  ///////////////////////////////////////////////////////////////
  else if( strcmp( tok1, ( const char * ) "scanner_mode" ) == 0 ) {

    val1 = command_is_one( tok2 ); //on/off 0/1
    if( val1 >= 0 && toks == 2 ) {
      config->scanner_mode = val1;
    } else if( toks == 2 ) {
      config->scanner_mode = ( int ) atoi( tok2 ); //more detailed logging

    }


    if( config->scanner_mode == 0 ) {
      set_frequency_900mhz( 954.5f );
      delay_ms( 5 );
      at86rf215_write_single( RF09_CMD, CMD_RF_RX );  //trxoff -> txprep does calibration
      delay_ms( 5 );
      printf( "\r\nset frequency to 954.5 MHz" );
    } else {
      config->srate = 800000;
      config->rfgain = 0;
      config->sa_mode = 0;
    }

    printf( "\r\nscanner_mode %d", config->scanner_mode );

  }
  ///////////////////////////////////////////////////////////////
  // P25_LOGGING
  ///////////////////////////////////////////////////////////////
  else if( strcmp( tok1, ( const char * ) "p25_logging" ) == 0 ) {

    val1 = command_is_one( tok2 ); //on/off 0/1
    if( val1 >= 0 && toks == 2 ) {
      config->p25_logging = val1;
    } else if( toks == 2 ) {
      config->p25_logging = ( int ) atoi( tok2 ); //more detailed logging
    }
    printf( "\r\np25_logging %d", config->p25_logging );

  }
  ///////////////////////////////////////////////////////////////
  // TESTFLASH
  ///////////////////////////////////////////////////////////////
  else if( strcmp( tok1, ( const char * ) "testflash" ) == 0 ) {
    do_test_flash = 1;
  }
  ///////////////////////////////////////////////////////////////
  // WRITE_CONFIG
  ///////////////////////////////////////////////////////////////
  else if( strcmp( tok1, ( const char * ) "write_config" ) == 0 ) {
    do_write_config = 1;
  }
  ///////////////////////////////////////////////////////////////
  // READ_CONFIG
  ///////////////////////////////////////////////////////////////
  else if( strcmp( tok1, ( const char * ) "read_config" ) == 0 ) {
    do_read_config = 1;
  }
  ///////////////////////////////////////////////////////////////
  // NET_ID
  ///////////////////////////////////////////////////////////////
  else if( strcmp( tok1, ( const char * ) "net_id" ) == 0 ) {
    if( toks >= 2 ) {
      config->p25_network_id = atoi( tok2 );
      if( toks == 3 ) config->p25_network_id_allow_all = atoi( tok3 );
      else config->p25_network_id_allow_all = 0; //setting id implies don't allow all
    }
    printf( "\r\np25_network_id = %d, allow all=%d", config->p25_network_id, config->p25_network_id_allow_all );
  }
  ///////////////////////////////////////////////////////////////
  // RADIOMANF_ID
  ///////////////////////////////////////////////////////////////
  else if( strcmp( tok1, ( const char * ) "radiomanf_id" ) == 0 ) {
    if( toks >= 2 ) {
      config->p25_radiomanf_id = atoi( tok2 );
      if( toks == 3 ) config->p25_radiomanf_id_allow_all = atoi( tok3 );
      else config->p25_radiomanf_id_allow_all = 0;
    }
    printf( "\r\np25_radiomanf_id = %d, allow all=%d", config->p25_radiomanf_id, config->p25_radiomanf_id_allow_all );
  }
  ///////////////////////////////////////////////////////////////
  // RESET
  ///////////////////////////////////////////////////////////////
  else if( strcmp( tok1, ( const char * ) "system_reset" ) == 0 ) {
    do_system_reset = 1;
    reset_timer = HAL_GetTick();
    printf( "\r\ninitiating system reset" );
  }
  ///////////////////////////////////////////////////////////////
  // HELP
  ///////////////////////////////////////////////////////////////
  else if( strcmp( tok1, ( const char * ) "help" ) == 0 ) {
    show_help();
  }
  ///////////////////////////////////////////////////////////////
  // SHOW_CONFIG
  ///////////////////////////////////////////////////////////////
  else if( strcmp( tok1, ( const char * ) "show_config" ) == 0 ) {
    show_config();
  }
  ///////////////////////////////////////////////////////////////
  // AT_DUMP
  ///////////////////////////////////////////////////////////////
  else if( strcmp( tok1, ( const char * ) "at_dump" ) == 0 ) {
    do_at_dump = 1;
  }
  ///////////////////////////////////////////////////////////////
  // AT_SCAN
  ///////////////////////////////////////////////////////////////
  else if( strcmp( tok1, ( const char * ) "at_scan" ) == 0 ) {
    do_at_scan = 1;
    do_at_scan_cont = 0;
    at_scan_timer = HAL_GetTick();

    if( toks > 1 ) {
      val1 = command_is_one( tok2 ); //on/off 0/1
      if( val1 ) do_at_scan_cont = 1;
      else do_at_scan_cont = 0;
    }
  }
  ///////////////////////////////////////////////////////////////
  //
  ///////////////////////////////////////////////////////////////
  else if( strcmp( tok1, ( const char * ) "tunertest" ) == 0 ) {
    do_tuner_test();
  } else if( strcmp( tok1, ( const char * ) "pll" ) == 0 ) {
    check_pll_lock();
  } else {
    if( toks > 0 ) printf( "\r\nunknown command. try 'help'" );
  }

  if( do_prompt ) print_prompt();
}
//////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////
int command_is_one( char *str )
{
  if( strcmp( str, "1" ) == 0 ) return 1;
  if( strcmp( str, "on" ) == 0 ) return 1;
  if( strcmp( str, "0" ) == 0 ) return 0;
  if( strcmp( str, "off" ) == 0 ) return 0;
  return -1;
}

//////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////
void command_tick( void )
{

  //handle testmem command from main loop
  if( do_test_mem ) {
    do_test_mem = 0;
    test_mem();
    print_prompt();
  }

  if( do_test_flash ) {
    do_test_flash = 0;
    test_flash( 0 );
    print_prompt();
  }

  if( do_write_config ) {
    do_write_config = 0;
    printf( "\r\nwriting configuration to ext flash." );
    write_configuration_to_flash();
    print_prompt();
  }

  if( do_read_config ) {
    do_read_config = 0;
    printf( "\r\nreading configuration from ext flash." );
    read_configuration_from_flash();
    print_prompt();
  }

  if( do_system_reset ) {
    if( HAL_GetTick() - reset_timer > 500 ) {
      close_telnet();
    }
    if( HAL_GetTick() - reset_timer > 1500 ) {
      NVIC_SystemReset();
    }
  }

  if( do_at_dump ) {
    do_at_dump = 0;
    atrf_dump_regs();
    print_prompt();
  }

  if( do_at_scan_cont ) {
    if( HAL_GetTick() - at_scan_timer > 200 ) {
      at_scan_timer = HAL_GetTick();
      do_at_scan = 1;
    }
  }

  if( do_at_scan ) {
    do_at_scan = 0;
    _do_channel_scan();
    if( !do_at_scan_cont ) print_prompt();
  }

  if( do_handle_exti1 ) {
    do_handle_exti1 = 0;
    //udp_send_response_iq900();  //this can't really wait much
  }

}

//////////////////////////////////////////////////////////////////////////////////////////////////
// TESTMEM
// AUDIO
// P25_LOGGING
// TESTFLASH
// WRITE_CONFIG
// READ_CONFIG
// NET_ID
// RADIOMANF_ID
// SYSTEM_RESET
// HELP
//////////////////////////////////////////////////////////////////////////////////////////////////
void show_help( void )
{
  printf( "\r\n " );
  printf( "\r\nat_dump" );
  printf( "\r\nat_scan [cont: 0/1]" );
  printf( "\r\naudio [vol 0.0 - 1.0] [on/off]" );
  printf( "\r\nhelp" );
  printf( "\r\nnet_id id_dec [always 0/1]" );
  printf( "\r\np25_logging [log_level_dec]" );
  printf( "\r\nradiomanf_id id_dec [always 0/1]" );
  printf( "\r\nshow_config" );
  printf( "\r\nsystem_reset" );
  printf( "\r\ntestflash" );
  printf( "\r\ntestmem" );
  printf( "\r\nwrite_config" );
  printf( "\r\n " );
}
//////////////////////////////////////////////////////////////////////////////////////////////////
void show_config( void )
{
  printf( "\r\naudio %d, audio vol %3.2f", config->audio_on, config->audio_volume_f );
  printf( "\r\np25_logging %d", config->p25_logging );
  printf( "\r\np25_network_id = %d, allow all=%d", config->p25_network_id, config->p25_network_id_allow_all );
  printf( "\r\np25_radiomanf_id = %d, allow all=%d", config->p25_radiomanf_id, config->p25_radiomanf_id_allow_all );
  printf( "\r\n " );
}



#define TUNER_DELAY 500
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void do_tuner_test( void )
{

  uint32_t freq = 705e6;

  printf( "\r\ntesting tuner spi port" );

  tuner_select();

  uint8_t *ptr = ( uint8_t * ) &freq;

  _xmit_tuner( ( uint8_t ) 0x06 ); //len
  _xmit_tuner( ( uint8_t ) TUNER_CMD_SET_FREQ );
  _xmit_tuner( ( uint8_t ) *ptr++ );
  _xmit_tuner( ( uint8_t ) *ptr++ );
  _xmit_tuner( ( uint8_t ) *ptr++ );
  _xmit_tuner( ( uint8_t ) *ptr++ );


  uint8_t rx = _xmit_tuner( ( uint8_t ) 0xff );

  if( rx == 0x55 ) {
    printf( "\r\nset frequency ok" );
  } else {
    printf( "\r\nno ack from tuner" );
  }

  tuner_deselect();

  delay_us( 2500 ); //wait for set freq to finish, must wait at least this long

  ///////////////////////////////////////////////////////

  tuner_select();

  ptr = ( uint8_t * ) &freq;

  _xmit_tuner( ( uint8_t ) 0x02 ); //len
  _xmit_tuner( ( uint8_t ) TUNER_CMD_GET_FREQ );

  *ptr++ = _xmit_tuner( 0xff );
  *ptr++ = _xmit_tuner( 0xff );
  *ptr++ = _xmit_tuner( 0xff );
  *ptr++ = _xmit_tuner( 0xff );

  tuner_deselect();
  printf( "\r\nfreq_hz: %u", freq );

  int cnt = 0;

  for( cnt = 0; cnt < 100; cnt++ ) {
    ///////////////////////////////////////////////////////
    tuner_select();

    _xmit_tuner( ( uint8_t ) 0x02 ); //len
    _xmit_tuner( ( uint8_t ) TUNER_CMD_PLL_STATUS );

    rx = _xmit_tuner( 0xff );

    tuner_deselect();


    if( ( rx & 0x04 ) == 4 ) break; //locked
  }
  if( ( rx & 0x04 ) == 4 ) printf( "\r\npll status: %u, cnt: %d", rx, cnt );
  else printf( "\r\npll did not lock, %02x", rx );


  ///////////////////////////////////////////////////////

  tuner_select();

  ptr = ( uint8_t * ) &freq;

  _xmit_tuner( ( uint8_t ) 0x02 ); //len
  _xmit_tuner( ( uint8_t ) TUNER_CMD_GET_LOW_FREQ );

  *ptr++ = _xmit_tuner( 0xff );
  *ptr++ = _xmit_tuner( 0xff );
  *ptr++ = _xmit_tuner( 0xff );
  *ptr++ = _xmit_tuner( 0xff );

  tuner_deselect();
  printf( "\r\nlow freq_hz: %u", freq );

  ///////////////////////////////////////////////////////

  tuner_select();

  ptr = ( uint8_t * ) &freq;

  _xmit_tuner( ( uint8_t ) 0x02 ); //len
  _xmit_tuner( ( uint8_t ) TUNER_CMD_GET_HIGH_FREQ );

  *ptr++ = _xmit_tuner( 0xff );
  *ptr++ = _xmit_tuner( 0xff );
  *ptr++ = _xmit_tuner( 0xff );
  *ptr++ = _xmit_tuner( 0xff );

  tuner_deselect();
  printf( "\r\nhigh freq_hz: %u", freq );

  ///////////////////////////////////////////////////////

  tuner_select();

  ptr = ( uint8_t * ) &freq;

  _xmit_tuner( ( uint8_t ) 0x02 ); //len
  _xmit_tuner( ( uint8_t ) TUNER_CMD_GET_LOW_IF );

  *ptr++ = _xmit_tuner( 0xff );
  *ptr++ = _xmit_tuner( 0xff );
  *ptr++ = _xmit_tuner( 0xff );
  *ptr++ = _xmit_tuner( 0xff );

  tuner_deselect();
  printf( "\r\nlow IF freq_hz: %u", freq );

  ///////////////////////////////////////////////////////

  tuner_select();

  ptr = ( uint8_t * ) &freq;

  _xmit_tuner( ( uint8_t ) 0x02 ); //len
  _xmit_tuner( ( uint8_t ) TUNER_CMD_GET_HIGH_IF );

  *ptr++ = _xmit_tuner( 0xff );
  *ptr++ = _xmit_tuner( 0xff );
  *ptr++ = _xmit_tuner( 0xff );
  *ptr++ = _xmit_tuner( 0xff );

  tuner_deselect();
  printf( "\r\nhigh IF freq_hz: %u", freq );

  ///////////////////////////////////////////////////////

  freq = 908250000;

  tuner_select();

  ptr = ( uint8_t * ) &freq;

  _xmit_tuner( ( uint8_t ) 0x06 ); //len
  _xmit_tuner( ( uint8_t ) TUNER_CMD_SET_IF );
  _xmit_tuner( ( uint8_t ) *ptr++ );
  _xmit_tuner( ( uint8_t ) *ptr++ );
  _xmit_tuner( ( uint8_t ) *ptr++ );
  _xmit_tuner( ( uint8_t ) *ptr++ );


  rx = _xmit_tuner( ( uint8_t ) 0xff );

  if( rx == 0x55 ) {
    printf( "\r\nset IF frequency ok" );
  } else {
    printf( "\r\nno ack from tuner" );
  }

  tuner_deselect();

  delay_us( 2500 ); //wait for set freq to finish, must wait at least this long


  ////////////////////////////////////////////////////////////////////////////
  tuner_select();

  ptr = ( uint8_t * ) &freq;

  _xmit_tuner( ( uint8_t ) 0x02 ); //len
  _xmit_tuner( ( uint8_t ) TUNER_CMD_GET_IF );

  *ptr++ = _xmit_tuner( 0xff );
  *ptr++ = _xmit_tuner( 0xff );
  *ptr++ = _xmit_tuner( 0xff );
  *ptr++ = _xmit_tuner( 0xff );

  tuner_deselect();
  printf( "\r\nIF freq_hz: %u", freq );

  cnt = 0;


  for( cnt = 0; cnt < 100; cnt++ ) {
    ///////////////////////////////////////////////////////
    tuner_select();

    _xmit_tuner( ( uint8_t ) 0x02 ); //len
    _xmit_tuner( ( uint8_t ) TUNER_CMD_PLL_STATUS );

    rx = _xmit_tuner( 0xff );

    tuner_deselect();


    if( ( rx & 0x04 ) == 4 ) break; //locked
  }
  if( ( rx & 0x04 ) == 4 ) printf( "\r\npll status: %u, cnt: %d", rx, cnt );
  else printf( "\r\npll did not lock: %02x", rx );


}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
void check_pll_lock()
{
  int cnt = 0;
  uint8_t rx;

  for( cnt = 0; cnt < 100; cnt++ ) {
    ///////////////////////////////////////////////////////
    tuner_select();

    _xmit_tuner( ( uint8_t ) 0x02 ); //len
    _xmit_tuner( ( uint8_t ) TUNER_CMD_PLL_STATUS );

    rx = _xmit_tuner( 0xff );

    tuner_deselect();


    if( ( rx & 0x04 ) == 4 ) break; //locked
  }
  if( ( rx & 0x04 ) == 4 ) printf( "\r\npll status: %u, cnt: %d", rx, cnt );
  else printf( "\r\npll did not lock, %02x", rx );
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
void tuner_select()
{
  //delay_us(4);  //don't wait
  HAL_GPIO_WritePin( GPIOC, GPIO_PIN_7, GPIO_PIN_RESET ); //fpga cs,  tuner cs
  HAL_GPIO_WritePin( GPIOG, GPIO_PIN_0, GPIO_PIN_RESET );
  delay_us( 5 );
}
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
void tuner_deselect()
{
  delay_us( 4 );
  HAL_GPIO_WritePin( GPIOC, GPIO_PIN_7, GPIO_PIN_SET ); //fpga cs,  tuner cs
  HAL_GPIO_WritePin( GPIOG, GPIO_PIN_0, GPIO_PIN_SET );
  delay_us( 4 );
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
uint8_t _xmit_tuner( uint8_t out_reg )
{

  uint8_t in_reg;
  uint8_t tx = out_reg;

  HAL_StatusTypeDef status;

  delay_us( 4 );
  status = HAL_SPI_TransmitReceive_IT( &hspi3, &tx, &in_reg, 1 );
  delay_us( 4 );

  return in_reg;
}
