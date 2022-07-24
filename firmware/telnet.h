
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



#ifndef __TELNET_H__
#define __TELNET_H__

#include "lwip/tcp.h"

#define CMD_BUFFER_LEN 1500
uint8_t cmd_buffer[CMD_BUFFER_LEN];

enum {
  CMD_RESP_NONE,
  CMD_RESP_TELNET
};

enum {
  INTF_SRC_NONE,
  INTF_SRC_ETH
};

void telnet_init( void );
void close_telnet( void );
void telnet_write( struct tcp_pcb *tn_write_pcb, uint8_t *buffer, int len );
err_t telnet_poll( void *arg, struct tcp_pcb *pcb );
void telnet_tick();
int is_telnet_closed();
void handle_command_telnet( char *cmd_buffer, int arg2, int interface, int cmd_idx, int arg5 );
void print_prompt( void );
#endif
