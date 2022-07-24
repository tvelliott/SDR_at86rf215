

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
#include <stdarg.h>
#include <stdio.h>
#include "std_io.h"
#include "usbd_cdc_vcp.h"
#include "global.h"
//#include "ethernet.h"
char acm_in_buffer[MAX_PRINTF_BUFFER];
char eem_buffer[MAX_PRINTF_BUFFER];

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
int acm_scanf( char * format, ...)
{
  int ret=0;

  va_list args;

  va_start(args,format);

  acm_in_buffer[MAX_PRINTF_BUFFER-1]=0;

  ret = vsscanf((char *) acm_in_buffer, format, args);

  va_end(args);

  return ret; //length sent to device
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
int printf( const char * format, ...)
{

  uint16_t ret=0;

  va_list args;

  char printf_buf[MAX_PRINTF_BUFFER];

  va_start(args,format);
  ret = vsnprintf((char *) printf_buf, sizeof(printf_buf)-1, format, args);
  printf_buf[MAX_PRINTF_BUFFER-1] = 0; //null terminate to user specified limit

  VCP_send_str( (uint8_t *) printf_buf );

  va_end(args);

  return ret; //length sent to device
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
int eem_printf( const char * format, ...)
{

  uint16_t ret=0;

  va_list args;

  va_start(args,format);
  ret = vsnprintf((char *) eem_buffer, sizeof(eem_buffer)-1, format, args);
  eem_buffer[MAX_PRINTF_BUFFER-1] = 0; //null terminate to user specified limit

  va_end(args);

  return ret; //length sent to device
}
