
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
#include <stdint.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include "std_io.h"

extern UART_HandleTypeDef huart3;
uint8_t net_buffer[MAX_NET_BUFFER];
uint8_t printf_buf[MAX_PRINTF_BUFFER];
extern int telnet_session_count;

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void printf_uart( char *net_buffer )
{

  if( net_buffer[0] != 0 ) {
    HAL_UART_Transmit( &huart3, net_buffer, strlen( ( char * )net_buffer ), HAL_MAX_DELAY );
    net_buffer[0] = 0;
  }

}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
/*
int _write(int file, char *ptr, int len) {
  //printf("\r\n_write called, file: %d, len: %d, ptr=%08x", file, len, ptr);
  //
  switch(file) {
    case 1  : //stdout
      HAL_UART_Transmit(&huart3, (uint8_t *) ptr, len, HAL_MAX_DELAY );
    break;

    case 2  : //stderr
      HAL_UART_Transmit(&huart3, (uint8_t *) ptr, len, HAL_MAX_DELAY );
    break;

    default :
      len = 0;
    break;
  }
  return len;
}

void _close(void) {
  printf("\r\n_close called");
}
void _fstat(void) {
  printf("\r\n_fstat called");
}
void _isatty(void) {
  printf("\r\n_isatty called");
}
void _lseek(void) {
  printf("\r\n_lseek called");
}
void _read(void) {
  printf("\r\n_read called");
}
void _kill(void) {
  printf("\r\n_kill called");
}
void _getpid(void) {
  printf("\r\n_getpid called");
}
void _open(void) {
  printf("\r\n_open called");
}
void exit(int n) {
  printf("\r\nexit called");
  while(1) {
  }
}
void abort(int n) {
  printf("\r\nabort called");
  while(1) {
  }
*/
int __io_putchar( int ch )
{
  HAL_UART_Transmit( &huart3, ( uint8_t * ) &ch, 1, HAL_MAX_DELAY );
  return 0;
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
int printf( const char *format, ... )
{

  uint16_t ret = 0;
  va_list args;



  va_start( args, format );

  ret = vsnprintf( printf_buf, sizeof( printf_buf ) - 1, format, args );
  printf_buf[MAX_PRINTF_BUFFER - 1] = 0;

  int len1 = strlen( net_buffer );
  int len2 = strlen( printf_buf );


  if( len1 + len2 < MAX_NET_BUFFER ) {
    strcat( net_buffer, printf_buf );
    //net_buffer[len1+len2]=0x00;
    net_buffer[MAX_NET_BUFFER - 1] = 0;
  }

  va_end( args );

  if( is_telnet_closed() ) {
    printf_uart( net_buffer );
    return 0;
  }

  return ret; //length sent to device
}
