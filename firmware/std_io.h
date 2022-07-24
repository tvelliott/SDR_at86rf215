
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



#ifndef STD_IO__H
#define STD_IO__H

#include "stm32h7xx_hal.h"
#include <stdarg.h>
#include <stdint.h>

#define MAX_PRINTF_BUFFER 1460*16
#define MAX_NET_BUFFER 1460*16

int printf( const char *format, ... );
int __io_putchar( int ch );
void printf_uart( char *net_buffer );
extern int telnet_session_count;
extern uint8_t printf_buf[MAX_PRINTF_BUFFER];
extern uint8_t net_buffer[MAX_NET_BUFFER];
extern UART_HandleTypeDef huart3;



#endif
