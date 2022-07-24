
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



#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <complex.h>
#include <string.h>
#include <float.h>
#include <complex.h>
#include "rtl_tcp_driver.h"

#include "lwip/opt.h"
#include "lwip/tcp.h"
#include "std_io.h"
#include "fpga.h"
#include "globals.h"

extern volatile int8_t iq_data_ready;
static struct tcp_pcb *rtl_tcp_driver_pcb;
/* copied from rtl sdr */
enum rtlsdr_tuner {
  RTLSDR_TUNER_UNKNOWN = 0,
  RTLSDR_TUNER_E4000,
  RTLSDR_TUNER_FC0012,
  RTLSDR_TUNER_FC0013,
  RTLSDR_TUNER_FC2580,
  RTLSDR_TUNER_R820T,
  RTLSDR_TUNER_R828D
};
static struct tcp_pcb *write_pcb;

static uint8_t *trace_ptr;
static int outstanding;
int do_get_trace;
int do_get_sa_trace;
int count = 0;

static int8_t data_f[512];

int poll_mod;
static double freq;

int do_gqrx = 0;
int prev_gain = 0;

volatile struct tcp_pcb *rtl_pcb;

int agc_is_on;
int first_agc = 1;

/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
void rtl_tcp_send( void )
{
  if( rtl_pcb != NULL && tcp_sndbuf( rtl_pcb ) > 2048 ) {
    tcp_write( rtl_pcb, ( uint8_t * )&iq_data[0], 512, 1 );
  }
};

/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
static err_t rtl_tcp_driver_write( void *arg, struct tcp_pcb *pcb )
{

  rtl_pcb = pcb;

#if 0
  if( do_gqrx && pcb != NULL ) {

    while( tcp_sndbuf( pcb ) > 2048 ) {

      while( !iq_data_ready ) { };
      iq_data_ready = 0;

      tcp_write( pcb, ( uint8_t * )&iq_data[0], 512, 1 );
    }
  }
#endif

  return ERR_OK;
}
//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////
static err_t rtl_tcp_driver_sent( void *arg, struct tcp_pcb *pcb, int len )
{
  rtl_pcb = pcb;
#if 0
  while( !iq_data_ready ) { };
  iq_data_ready = 0;
  tcp_write( pcb, ( uint8_t * )&iq_data[0], 512, 1 );
#endif
  return ERR_OK;
}

//////////////////////////////////////////////////////////////////////////////////////////////
static err_t rtl_tcp_driver_recv( void *arg, struct tcp_pcb *pcb, struct pbuf *p, err_t err )
{
  int i;

  if( err == ERR_OK && p != NULL ) {

    tcp_recved( pcb, p->tot_len );

    //
    char *ptr = p->payload;
    char *ptr2;
    //ptr[p->tot_len] = 0;

    struct pbuf *q = p;
    int len = q->len;
    ptr = q->payload;
    int bytes_out = 0;

    uint32_t value;
    int n;
    uint8_t cmd_mod;

    while( q ) {
      //add payload to serial buffer
      //printf("\r\nRX: ");
      for( i = 0; i < len; i++ ) {

        cmd_mod++;
        cmd_mod %= 5;

        if( cmd_mod == 0 && *ptr == 0x03 ) {
          memcpy( &value, ( ptr + 1 ), 4 );
          value = htonl( value );



          tcp_write( pcb, ptr, 5, 1 );
          tcp_output( pcb );
          ptr += 5;
          i += 5;
        }

        //set gain
        else if( cmd_mod == 0 && *ptr == 0x04 ) {

          memcpy( &value, ( ptr + 1 ), 4 );
          value = htonl( value );
          value += 10; //0-430

          //float gain = (float) ((double) value)/18.695652f;
          //printf("\r\ngain: %3.1f", value);
          value /= 19;
          value += 1;
          printf( "\r\ngain: %d", value );

          int g = ( int ) value;

          if( g < 0 ) g = 0;
          if( g > 23 ) g = 23;

          at86rf215_set_gain( ( int ) g );
          prev_gain = g;

          tcp_write( pcb, ptr, 5, 1 );
          tcp_output( pcb );
          ptr += 5;
          i += 5;

        }

        //set frequency
        else if( cmd_mod == 0 && *ptr == 0x01 ) {
          memcpy( &value, ( ptr + 1 ), 4 );
          value = htonl( value );
          tuner_set_freq_hz( value );

          printf( "\r\nfrequency: %u", value );

          if( first_agc ) {
            at86rf215_set_gain( 0 );
            printf( "\r\nagc on" );
            agc_is_on = 1;
            first_agc = 0;
          }

          tcp_write( pcb, ptr, 5, 1 );
          tcp_output( pcb );
          ptr += 5;
          i += 5;
        }

        //set sampling rate
        else if( cmd_mod == 0 && *ptr == 0x02 ) {
          memcpy( &value, ( ptr + 1 ), 4 );
          value = htonl( value );

          printf( "\r\nsampling rate: %u", value );

          tcp_write( pcb, ptr, 5, 1 );
          tcp_output( pcb );
          ptr += 5;
          i += 5;
        }
        //set IF gain
        else if( cmd_mod == 0 && *ptr == 0x06 ) {
          memcpy( &value, ( ptr + 1 ), 4 );
          value = htonl( value );


          tcp_write( pcb, ptr, 5, 1 );
          tcp_output( pcb );
          ptr += 5;
          i += 5;
        }
        //AGC
        else if( cmd_mod == 0 && *ptr == 0x08 ) {
          memcpy( &value, ( ptr + 1 ), 4 );
          value = htonl( value );

          if( value == 1 ) {
            at86rf215_set_gain( 0 );
            printf( "\r\nagc on" );
            agc_is_on = 1;
          } else {
            printf( "\r\nagc off" );
            at86rf215_set_gain( ( int ) prev_gain );
            agc_is_on = 0;
          }


          tcp_write( pcb, ptr, 5, 1 );
          tcp_output( pcb );
          ptr += 5;
          i += 5;
        }
        //freq error in PPM
        else if( cmd_mod == 0 && *ptr == 0x05 ) {
          memcpy( &value, ( ptr + 1 ), 4 );
          value = htonl( value );


          tcp_write( pcb, ptr, 5, 1 );
          tcp_output( pcb );
          ptr += 5;
          i += 5;
        }
        //Unknown, but so far only 0 on startup of gqrx only
        else if( cmd_mod == 0 && *ptr == 0x09 ) {
          memcpy( &value, ( ptr + 1 ), 4 );
          value = htonl( value );


          tcp_write( pcb, ptr, 5, 1 );
          tcp_output( pcb );
          ptr += 5;
          i += 5;
        }

        else if( cmd_mod == 0 ) {
          //printf("\r\nunknown: len: %d,  cmd: %04x", len, *ptr);

          ptr2 = ptr;
          printf( "\r\nunknown cmdx_dump: " );
          for( n = 0; n < 5; n++ ) {
            printf( "%02x, ", *ptr2++ );
          }

          tcp_write( pcb, ptr, 5, 1 );
          tcp_output( pcb );
          ptr += 5;
          i += 5;
        }

      }
      q = q->next;
      if( q != NULL ) {
        len = q->len;
        ptr = q->payload;
        cmd_mod = 0;
      }
    }

    //printf("\r\nRESP: len=%d", p->tot_len);
    //tcp_write(pcb, "\r\n\0x00\0x00\0x00\0x00", p->tot_len,1);
    //tcp_output(pcb);

  }

  pbuf_free( p );

  if( err == ERR_OK && p == NULL ) {
    tcp_arg( pcb, NULL );
    tcp_sent( pcb, NULL );
    tcp_recv( pcb, NULL );
    tcp_close( pcb );
    write_pcb = NULL;
    outstanding = 0;
    do_gqrx = 0;
    //restore settings
    at86rf215_set_srate( config->srate );
    at86rf215_set_gain( config->rfgain );
    first_agc = 1;
  }


  return ERR_OK;
}

//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////
static err_t rtl_tcp_driver_accept( void *arg, struct tcp_pcb *pcb, err_t err )
{
  LWIP_UNUSED_ARG( arg );
  LWIP_UNUSED_ARG( err );

  write_pcb = pcb;

  tcp_arg( pcb, NULL );
  tcp_sent( pcb, rtl_tcp_driver_sent );
  tcp_recv( pcb, rtl_tcp_driver_recv );
  //tcp_poll(pcb, rtl_tcp_driver_poll,1);


  /* copied from rtl sdr code */
  typedef struct { /* structure size must be multiple of 2 bytes */
    char magic[4];
    uint32_t tuner_type;
    uint32_t tuner_gain_count;
  } dongle_info_t;

  dongle_info_t dongle_info;

  memcpy( &dongle_info.magic[0], "RTL0", 4 );
  dongle_info.tuner_type = htonl( RTLSDR_TUNER_E4000 );
  dongle_info.tuner_gain_count = htonl( 23 );

  tcp_write( pcb, &dongle_info, sizeof( dongle_info_t ), 1 );
  tcp_output( pcb );

  do_gqrx = 1;
  at86rf215_set_srate( 2000000 );
  config->sa_mode = 4;
  at86rf215_set_gain( 0 );

  tcp_write( pcb, &data_f, 1024, 1 );
  tcp_output( pcb );

  return ERR_OK;
}

//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////
void rtl_tcp_driver_init( void )
{

  rtl_tcp_driver_pcb = tcp_new();
  tcp_bind( rtl_tcp_driver_pcb, IP_ADDR_ANY, 1234 );
  rtl_tcp_driver_pcb = tcp_listen( rtl_tcp_driver_pcb );
  tcp_accept( rtl_tcp_driver_pcb, rtl_tcp_driver_accept );

}

//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////
void tuner_set_freq_hz( uint32_t freq )
{
  uint32_t _freq = freq;

  tuner_select();

  uint8_t *ptr = ( uint8_t * ) &_freq;

  _xmit_tuner( ( uint8_t ) 0x06 ); //len
  _xmit_tuner( ( uint8_t ) TUNER_CMD_SET_FREQ );
  _xmit_tuner( ( uint8_t ) *ptr++ );
  _xmit_tuner( ( uint8_t ) *ptr++ );
  _xmit_tuner( ( uint8_t ) *ptr++ );
  _xmit_tuner( ( uint8_t ) *ptr++ );


  uint8_t rx = _xmit_tuner( ( uint8_t ) 0xff );

  if( rx == 0x55 ) {
    //printf("\r\nset frequency ok");
  } else {
    //printf("\r\nno ack from tuner");
  }

  tuner_deselect();
}
