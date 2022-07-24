

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

#include <stdarg.h>
#include <stdint.h>

#define STDOUT_DEV_ACM  1
#define STDOUT_DEV_EEM  2
#define STDOUT_DEV_CC 3
#define STDOUT_DEV_DEBUG 4
#define STDOUT_DEV_USER_BUFFER 5

#define STDIN_DEV_ACM 32
#define STDIN_DEV_EEM 33
#define STDIN_DEV_CC  34
#define STDIN_DEV_DEBUG 35
#define STDIN_DEV_USER_BUFFER 36

#define MAX_PRINTF_BUFFER 2048

int eem_printf( const char * format, ...);
int acm_scanf( char * format, ...);
extern char acm_in_buffer[MAX_PRINTF_BUFFER];
extern char eem_buffer[MAX_PRINTF_BUFFER];

#endif
