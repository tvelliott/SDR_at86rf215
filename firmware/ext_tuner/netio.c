

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
#include <string.h>
#include "stm32f4_discovery.h"

#include "netio.h"

#include "lwip/opt.h"
#include "lwip/tcp.h"
#include "std_io.h"
#include "global.h"

#if LWIP_TCP
static err_t netio_recv(void *arg, struct tcp_pcb *pcb, struct pbuf *p, err_t err)
{
  int16_t sample_index;
  int16_t i;
  uint32_t time;

  LWIP_UNUSED_ARG(arg);

  if (err == ERR_OK && p != NULL) {
    tcp_recved(pcb, p->tot_len);

    char *ptr = p->payload;
    ptr[p->tot_len] = 0;

    //if( memcmp(ptr,"trace",5) == 0 ) {
    //  err_t wr_err = tcp_write(pcb, samples, 2*(2*1000), 1);
    //}

  }

  pbuf_free(p);

  if (err == ERR_OK && p == NULL) {
    tcp_arg(pcb, NULL);
    tcp_sent(pcb, NULL);
    tcp_recv(pcb, NULL);
    tcp_close(pcb);
  }

  return ERR_OK;
}

static err_t netio_accept(void *arg, struct tcp_pcb *pcb, err_t err)
{
  LWIP_UNUSED_ARG(arg);
  LWIP_UNUSED_ARG(err);

  tcp_arg(pcb, NULL);
  tcp_sent(pcb, NULL);
  tcp_recv(pcb, netio_recv);
  return ERR_OK;
}

void netio_init(void)
{
  struct tcp_pcb *pcb;

  pcb = tcp_new();
  tcp_bind(pcb, IP_ADDR_ANY, 9997);
  pcb = tcp_listen(pcb);
  tcp_accept(pcb, netio_accept);
}

#endif /* LWIP_TCP */
