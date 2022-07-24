
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
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

#include "rle.h"

uint8_t *comp_in;
uint8_t *comp_out;
static uint8_t *inptr;
static uint8_t *outptr;
static uint8_t *ptr;

////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////
void init_rle( void )
{
  if( comp_in == NULL ) comp_in = malloc( COMP_LEN );
  if( comp_out == NULL ) comp_out = malloc( COMP_LEN );
}

////////////////////////////////////////////////////////////////////////////////////////////
// run-length encoding
////////////////////////////////////////////////////////////////////////////////////////////
int rle_encode( char *in, char *out, int len )
{

  int currChar, prevChar;
  unsigned char count;


  prevChar = RLE_NULL;
  count = 0;
  int index = 0;
  int out_bytes = 0;


  while( index < len + 1 ) {
    currChar = *in++;
    index++;

    *out++ = ( uint8_t )( currChar & 0xff );
    out_bytes++;


    if( currChar == prevChar ) {

      count = 0;

      while( index < len + 1 ) {

        currChar = *in++;
        index++;

        if( currChar == prevChar ) {
          count++;

          if( count == 255 ) {

            *out++ = count;
            out_bytes++;


            prevChar = RLE_NULL;
            break;
          }
        } else {

          *out++ = count;
          out_bytes++;
          *out++ = ( uint8_t )( currChar & 0xff );
          out_bytes++;
          prevChar = currChar;
          break;
        }
      }
    } else {

      prevChar = currChar;
    }

    if( index == len + 1 ) {

      *out++ = count;
      out_bytes++;
      break;
    }
  }

  return out_bytes;
}

////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////
int rle_decode( char *in, char *out, int len )
{

  int currChar, prevChar;
  unsigned char count;


  prevChar = RLE_NULL;
  int out_bytes = 0;


  while( out_bytes < len + 1 ) {

    currChar = *in++;
    *out++ = ( uint8_t )( currChar & 0xff );
    out_bytes++;


    if( currChar == prevChar ) {

      count = *in++;
      while( count > 0 ) {
        *out++ = ( uint8_t )( currChar & 0xff );
        out_bytes++;
        count--;
      }

      prevChar = RLE_NULL;
    } else {

      prevChar = currChar;
    }
  }

  return out_bytes - 1;
}
