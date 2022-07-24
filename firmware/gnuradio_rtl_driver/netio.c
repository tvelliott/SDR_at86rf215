#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <complex.h>
#include <string.h>
#include <float.h>
#include <complex.h>
#include "netio.h"

#include "lwip/opt.h"
#include "lwip/tcp.h"
#include "std_io.h"
#include "config.h"
#include "fpga.h"
#include "serial_data_interface.h"
#include "fpga.h"
#include "global.h"
#include "at86rf215.h"

static struct tcp_pcb *netio_pcb;
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
int count=0;

static int16_t data_f[512];



int poll_mod;
static double freq;

/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
static err_t netio_write(void *arg, struct tcp_pcb *pcb) {

static int16_t *ptr_i1 = 0x60004000;
static int16_t *ptr_q1 = 0x60005000;
static int16_t *ptr_i2 = 0x60006000;
static int16_t *ptr_q2 = 0x60007000;

int16_t udp_data_i[256] __attribute__ ((aligned (4))); 
int16_t udp_data_q[256] __attribute__ ((aligned (4))); 


  if(pcb!=NULL) {

      while( tcp_sndbuf(pcb) > 2000) {

        while(!do_handle_exti0) { };
        do_handle_exti0=0;

        if(!fifo0_900_ready) {
          memcpy((uint8_t *) udp_data_i, ptr_i1, 512);
          memcpy((uint8_t *) udp_data_q, ptr_q1, 512);
        }
        else {
          memcpy((uint8_t *) udp_data_i, ptr_i2, 512);
          memcpy((uint8_t *) udp_data_q, ptr_q2, 512);
        }

        int ii=0;
        for(int i=0; i<256; i++) {
          data_f[ii++] = udp_data_i[i];
          data_f[ii++] = udp_data_q[i];
        }

        tcp_write(pcb, (uint8_t *)&data_f[0], 512*2,1);
      }
  }

    return ERR_OK;
}
//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////
static err_t netio_sent(void *arg, struct tcp_pcb *pcb, int len) {
    if(len > 500 ) netio_write(arg, pcb);
    return ERR_OK;
}

//////////////////////////////////////////////////////////////////////////////////////////////
static err_t netio_recv(void *arg, struct tcp_pcb *pcb, struct pbuf *p, err_t err)
{
  int i;

  if (err == ERR_OK && p != NULL) {

    tcp_recved(pcb, p->tot_len);

    //
      char *ptr = p->payload;
      //ptr[p->tot_len] = 0;

      struct pbuf *q=p;
      int len = q->len;
      ptr = q->payload;
      int bytes_out=0;

      uint32_t value;

      while(q) {
        //add payload to serial buffer
        //printf("\r\nRX: ");
        for(i=0; i<len; i++) {

            //set gain
          if(i==0 && len == 5 && *ptr==0x04) { 
            memcpy(&value, (ptr+1), 4);
            value = htonl(value);

            //printf("\r\ngain: %u", value);

             //tcp_write(pcb,ptr,5,1);
             //tcp_output(pcb);

          }

          //set frequency
          if(i==0 && len == 5 && *ptr==0x01) { 
            memcpy(&value, (ptr+1), 4);
            value = htonl(value);
            freq = (double) value / 1e6;
            set_frequency_900mhz(freq); 

            //printf("\r\nfrequency: %u", value);
             tcp_write(pcb,ptr,5,1);
             tcp_output(pcb);
          }

          //set sampling rate
          if(i==0 && len >= 5 && *ptr==0x02) { 
            memcpy(&value, (ptr+1), 4);
            value = htonl(value);

            //printf("\r\nsampling rate: %u", value);

             //tcp_write(pcb,ptr,5,1);
             //tcp_output(pcb);
          }

        }
        q = q->next;
        if(q!=NULL) {
          len=q->len;
          ptr = q->payload;
        }
      }

    //printf("\r\nRESP: len=%d", p->tot_len);
    //tcp_write(pcb, "\r\n\0x00\0x00\0x00\0x00", p->tot_len,1);
    //tcp_output(pcb);

  }

  pbuf_free(p);

  if (err == ERR_OK && p == NULL) {
    tcp_arg(pcb, NULL);
    tcp_sent(pcb, NULL);
    tcp_recv(pcb, NULL);
    tcp_close(pcb);
    write_pcb = NULL;
    outstanding=0;
  }


  return ERR_OK;
}

//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////
static err_t netio_accept(void *arg, struct tcp_pcb *pcb, err_t err)
{
  LWIP_UNUSED_ARG(arg);
  LWIP_UNUSED_ARG(err);

  write_pcb = pcb;

  tcp_arg(pcb, NULL);
  tcp_sent(pcb, netio_sent);
  tcp_recv(pcb, netio_recv);
  //tcp_poll(pcb, netio_poll,1);


  /* copied from rtl sdr code */
  typedef struct { /* structure size must be multiple of 2 bytes */
      char magic[4];
        uint32_t tuner_type;
          uint32_t tuner_gain_count;
  } dongle_info_t;

   dongle_info_t dongle_info;

   memcpy(&dongle_info.magic[0], "EST0", 4);
   dongle_info.tuner_type=htonl(RTLSDR_TUNER_E4000);
   dongle_info.tuner_gain_count=htonl(77);

   tcp_write(pcb,&dongle_info,sizeof(dongle_info_t),1);
   tcp_output(pcb);

   tcp_write(pcb,&data_f,1024,1);
   tcp_output(pcb);

  return ERR_OK;
}

//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////
void netio_init(void)
{

  netio_pcb = tcp_new();
  tcp_bind(netio_pcb, IP_ADDR_ANY, 1234);
  netio_pcb = tcp_listen(netio_pcb);
  tcp_accept(netio_pcb, netio_accept);

}
