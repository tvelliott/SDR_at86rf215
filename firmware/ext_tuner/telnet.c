

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


#include <stdint.h>
#include "lwip/debug.h"
#include "lwip/stats.h"
#include "lwip/tcp.h"
#include "stm32f4_discovery.h"

#include <string.h>
#include <stdio.h>
#include "std_io.h"
#include "telnet.h"


#define TELNET_IAC   255
#define TELNET_WILL  251
#define TELNET_WONT  252
#define TELNET_DO    253
#define TELNET_DONT  254

#define IDLE_TIMEOUT  300

#define TELNET_FLAG_CLOSE_CONNECTION  (1<<0)

#define STATE_NORMAL 0
#define STATE_IAC    1
#define STATE_WILL   2
#define STATE_WONT   3
#define STATE_DO     4
#define STATE_DONT   5
#define STATE_CLOSE  6

typedef struct __attribute__((packed)) {
  char    *data_out;
  char    cmd_buffer[80];
  u16_t   left;
  u16_t   timeout;
  u16_t   flags;
  u16_t   state;
  int16_t   command_buffer_index;
  char tstr[2000];
} telnet_state;


const char prompt[] = {"\r\n>"};

// telnet command handler
void cmd_parser(telnet_state *hs)
{
  char *p = hs->cmd_buffer;

  if( memcmp(p, "quit", 4) == 0 || memcmp(p,"exit",4)==0 ) {
    hs->left = 0;
    hs->command_buffer_index=0;
    hs->flags |= TELNET_FLAG_CLOSE_CONNECTION;
  } else {
    eem_printf("\r\nunknown command: \"%s\"\r\n%s\0",p,prompt);
    strncpy(hs->tstr, eem_buffer, sizeof(hs->tstr) );
    hs->data_out = hs->tstr;
    hs->left = strlen(hs->tstr);
    hs->command_buffer_index=0;
  }

  hs->cmd_buffer[0] = 0;
}




/*-----------------------------------------------------------------------------------*/
static void conn_err(void *arg, err_t err)
{
  struct http_state *hs;
  //LWIP_UNUSED_ARG(err);
  hs = arg;
  if(hs!=NULL) mem_free(hs);
}

/*-----------------------------------------------------------------------------------*/
static void close_conn(struct tcp_pcb *pcb, telnet_state *hs)
{
  tcp_arg(pcb, NULL);
  tcp_sent(pcb, NULL);
  tcp_recv(pcb, NULL);
  tcp_close(pcb);
  if(hs!=NULL) mem_free(hs);
}

/*-----------------------------------------------------------------------------------*/
static void send_data(struct tcp_pcb *pcb, telnet_state *hs)
{
  err_t err;
  u16_t len;

  /* We cannot send more data than space available in the send
  buffer. */
  if (tcp_sndbuf(pcb) < hs->left) {
    len = tcp_sndbuf(pcb);
  } else {
    len = hs->left;
  }

  do {
    err = tcp_write(pcb, hs->data_out, len, 0);
    if (err == ERR_MEM) {
      len /= 2;
    }
  } while (err == ERR_MEM && len > 1);

  if (err == ERR_OK) {
    hs->data_out += len;
    hs->left -= len;
  }
}

/*-----------------------------------------------------------------------------------*/
static err_t telnet_poll(void *arg, struct tcp_pcb *pcb)
{
  telnet_state *hs;
  hs = arg;

  //printf("\r\nPoll");


  if (hs == NULL) {
    tcp_abort(pcb);
    return ERR_ABRT;
  } else {

    if (hs->flags & TELNET_FLAG_CLOSE_CONNECTION) {
      //printf("\r\nclosing connection.");
      close_conn(pcb, hs);
      hs->flags &= ~TELNET_FLAG_CLOSE_CONNECTION;
    }

    // timeout and close connection if nothing has been received for N seconds
    hs->timeout++;
    if (hs->timeout > IDLE_TIMEOUT)
      close_conn(pcb,hs);
  }
  return ERR_OK;
}

/*-----------------------------------------------------------------------------------*/
static err_t telnet_sent(void *arg, struct tcp_pcb *pcb, u16_t len)
{
  telnet_state *hs;

  //LWIP_UNUSED_ARG(len);

  hs = arg;

  if (hs->left > 0) {
    send_data(pcb, hs);
  } else {
    if (hs->flags & TELNET_FLAG_CLOSE_CONNECTION) {
      //printf("\r\nclosing connection");
      close_conn(pcb, hs);
      hs->flags &= ~TELNET_FLAG_CLOSE_CONNECTION;
    }
  }

  return ERR_OK;
}

void sendopt(telnet_state *hs, u8_t option, u8_t value)
{
  hs->data_out[hs->left++] = TELNET_IAC;
  hs->data_out[hs->left++] = option;
  hs->data_out[hs->left++] = value;
  hs->data_out[hs->left] = 0;
}

/*-----------------------------------------------------------------------------------*/
void get_char(struct tcp_pcb *pcb, telnet_state *hs, char c)
{
  hs->cmd_buffer[hs->command_buffer_index++] = c;
  hs->cmd_buffer[hs->command_buffer_index] = 0;

  if (c != '\n' && c!='\r' ) {
    if(hs->command_buffer_index >= sizeof(hs->cmd_buffer) ) {
      hs->command_buffer_index=0;
    }
  } else {
    hs->cmd_buffer[hs->command_buffer_index-1] = 0;
    if(c=='\r') {

      cmd_parser(hs);   // handle command
      if (hs->left > 0) {
        send_data(pcb, hs);
      }
    }
  }
}

/*-----------------------------------------------------------------------------------*/
static err_t telnet_recv(void *arg, struct tcp_pcb *pcb, struct pbuf *p, err_t err)
{
  telnet_state *hs;
  char *q,c;
  int len;

  hs = arg;

  if (err == ERR_OK && p != NULL) {

    /* Inform TCP that we have taken the data. */
    tcp_recved(pcb, p->tot_len);
    hs->timeout = 0;  // reset idletime ount
    hs->command_buffer_index=0;

    q = p->payload;
    len = p->tot_len;

    while(len > 0) {
      c = *q++;
      len--;

      switch (hs->state) {
      case STATE_IAC:
        if(c == TELNET_IAC) {
          //get_char(pcb,hs,c);
          hs->state = STATE_NORMAL;
        } else {
          switch(c) {
          case TELNET_WILL:
            hs->state = STATE_WILL;
            break;
          case TELNET_WONT:
            hs->state = STATE_WONT;
            break;
          case TELNET_DO:
            hs->state = STATE_DO;
            break;
          case TELNET_DONT:
            hs->state = STATE_DONT;
            break;
          default:
            hs->state = STATE_NORMAL;
            break;
          }
        }
        break;

      case STATE_WILL:
        /* Reply with a DONT */
        sendopt(hs,TELNET_DONT, c);
        hs->state = STATE_NORMAL;
        break;
      case STATE_WONT:
        /* Reply with a DONT */
        sendopt(hs,TELNET_DONT, c);
        hs->state = STATE_NORMAL;
        break;
      case STATE_DO:
        /* Reply with a WONT */
        sendopt(hs,TELNET_WONT, c);
        hs->state = STATE_NORMAL;
        break;
      case STATE_DONT:
        /* Reply with a WONT */
        sendopt(hs,TELNET_WONT, c);
        hs->state = STATE_NORMAL;
        break;
      case STATE_NORMAL:
        if(c == TELNET_IAC) {
          hs->state = STATE_IAC;
        } else {
          get_char(pcb,hs,c);
        }
        break;
      }
    }

    pbuf_free(p);
  }

  if (err == ERR_OK && p == NULL) {
    //printf("\r\nremote host closed connection");
    close_conn(pcb, hs);
  }

  return ERR_OK;
}

/*-----------------------------------------------------------------------------------*/
static err_t telnet_accept(void *arg, struct tcp_pcb *pcb, err_t err)
{

  //LWIP_UNUSED_ARG(arg);
  //LWIP_UNUSED_ARG(err);

  tcp_setprio(pcb, TCP_PRIO_MIN);

  telnet_state *hs;
  hs = (telnet_state *)mem_malloc(sizeof(telnet_state));

  //printf("\r\nnew connection (%08x) from %d.%d.%d.%d", hs, (pcb->remote_ip.addr >> 0) & 0xff, (pcb->remote_ip.addr >> 8) & 0xff, (pcb->remote_ip.addr >> 16) & 0xff, (pcb->remote_ip.addr >> 24) & 0xff);


  /* Initialize the structure. */
  memset(hs,0x00,sizeof(telnet_state));
  //memset(hs->tstr,0x00,sizeof(hs->tstr));
  //memset(hs->cmd_buffer,0x00,sizeof(hs->cmd_buffer));

  hs->timeout = 0;
  hs->flags = 0;
  hs->state = STATE_NORMAL;
  hs->command_buffer_index = 0;

  /* Tell TCP that this is the structure we wish to be passed for our
  callbacks. */
  tcp_arg(pcb, hs);

  /* Tell TCP that we wish to be informed of incoming data by a call
  to the http_recv() function. */
  tcp_recv(pcb, telnet_recv);

  tcp_err(pcb, conn_err);
  tcp_poll(pcb, telnet_poll, 1);
  tcp_sent(pcb, telnet_sent);

  eem_printf("\r\ntelnetd STM32F4xx, Cortex M4 @ 168 MHz\r\n%s",prompt);
  strncpy(hs->tstr, eem_buffer, sizeof(hs->tstr) );
  hs->data_out = hs->tstr;
  hs->left = strlen(hs->tstr);
  send_data(pcb, hs);

  return ERR_OK;
}

/**
 * Initialize telnet by setting up a raw connection to
 * listen on port 23
 *
 */
void telnet_init(void)
{
  struct tcp_pcb *pcb;

  pcb = tcp_new();
  tcp_bind(pcb, IP_ADDR_ANY, 23);
  pcb = tcp_listen(pcb);
  tcp_accept(pcb, telnet_accept);
}

