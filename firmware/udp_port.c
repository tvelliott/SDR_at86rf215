
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



#include "global.h"
#include "std_io.h"
#include "command_handler.h"
#include "udp_port.h"
#include "channel_filter.h"
#include "at86rf215.h"

static struct udp_pcb udp_ret_pcb;
static struct ip_addr udp_ret_addr;
static uint16_t udp_ret_port;
uint8_t udp_in_buffer[RF_FRAME_BUFFER_SIZE];
sdr_header_packet *sdr_p;
sdr_data_frame *sdr_frame;
static float bb_dec_gain;

//////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////
void udp_command_rx( void *arg, struct udp_pcb *pcb, struct pbuf *p, struct ip_addr *addr, uint16_t port )
{

  int index = 0;
  int i;
  uint8_t *ptr;
  int tot_len;

  if( p != NULL ) {

    memcpy( ( char * )&udp_ret_pcb, ( char * )pcb, sizeof( struct udp_pcb ) );
    memcpy( ( char * )&udp_ret_addr, ( char * )addr, sizeof( struct ip_addr ) );

    udp_ret_port = 8889;

    struct pbuf *q = p;
    int len = q->len;
    ptr = q->payload;

    while( q ) {
      //add payload to serial buffer
      for( i = 0; i < len; i++ ) {
        udp_in_buffer[index++] = *ptr++;
      }
      q = q->next;
      if( q != NULL ) {
        len = q->len;
        ptr = q->payload;
      }
    }

    tot_len = p->tot_len;

    sdr_frame = ( sdr_data_frame * ) udp_in_buffer;
    sdr_p = ( sdr_header_packet * ) &sdr_frame->header;
    bb_dec_gain = sdr_p->bb_dec_gain;

    if( sdr_p->magic != 0x23986544 ) goto rx_end;

    if( sdr_p->debug_mode ) {
      printf( "\r\nudp len: %d", tot_len );
      printf( "\r\ndata len: %d", sdr_p->data_len );
      printf( "\r\nmagic: %08x", sdr_p->magic );
      printf( "\r\nfreq: %d hz", sdr_p->freq_hz );
      printf( "\r\nbb_stddev: %3.4f", sdr_p->bb_stddev );
    }

    if( sdr_p->freq_hz != current_freq_hz ) {
      //current_freq_hz = sdr_p->freq_hz;
      //freq_d = (double) ((double) sdr_p->freq_hz) / 1000000.0;
      //set_frequency_900mhz(freq_d);
      //at86rf215_write_single( RF09_CMD, CMD_RF_RX );  //trxoff -> txprep does calibration
      //current_op_mode=SDR_OP_MODE_NONE;
    }

    if( sdr_p->command == SDR_COMMAND_RX ) {
      //if(current_op_mode!=SDR_OP_MODE_RX) {
      //  current_op_mode = SDR_OP_MODE_RX;
      //  set_frequency_900mhz(freq_d);
      //  at86rf215_write_single( RF09_CMD, CMD_RF_RX );  //trxoff -> txprep does calibration
      //}
    }

rx_end:

    //KEEP THIS HERE
    pbuf_free( p );

    //terminate
    udp_in_buffer[tot_len] = 0;

  }
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
void udp_send_response( uint8_t *buffer, int rlen )
{

  /*
  struct pbuf *p;

  if(rlen>0 && rlen<1450 && udp_ret_port!=0) {
    //one big chunk
    p = pbuf_alloc(PBUF_TRANSPORT, rlen, PBUF_RAM);
    if(p!=NULL) {
      memcpy(p->payload, buffer, rlen);
      net_buffer[0]=0;

      //send back response
      udp_sendto(&udp_ret_pcb, p, &udp_ret_addr, udp_ret_port);
      pbuf_free(p);
    }
  }
  */

}

////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////
void udp_cmd_port_init( void )
{
  struct udp_pcb *pcb;

  pcb = udp_new();
  if( pcb == NULL ) {
    return;
  }

  //command port 8889
  if( udp_bind( pcb, IP_ADDR_ANY, 8889 ) != ERR_OK ) {
    return;
  }

  udp_recv( pcb, udp_command_rx, NULL );
}
