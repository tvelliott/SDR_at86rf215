/*
//
//t.elliott converted from original liquid-dsp code to something that would be usable on the STM32H7 series
//
 * Copyright (c) 2007 - 2016 Joseph Gaeddert
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

//
// Windows, defined by macro
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <complex.h>
#include "window_cf_arm32.h"
#include "msb_index_arm32.h"

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// create window buffer object of length _n
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
struct window_cf_s *window_cf_create( struct window_cf_s *q, unsigned int _n )
{
  // set internal parameters
  q->len  = _n;                   // nominal window size
  q->m    = liquid_msb_index( _n ); // effectively floor(log2(len))+1
  q->n    = 1 << ( q->m );        // 2^m
  q->mask = q->n - 1;             // bit mask


  // number of elements to allocate to memory
  q->num_allocated = q->n + q->len - 1;
	//q->num_allocated = SYMSYNC_WINDOW_V_SIZE; 
	q->v = malloc( q->num_allocated * sizeof(float complex) );

  //printf("\r\nwindow array size %d", q->num_allocated);

  // allocte memory
  //q->v = farray;  //move this to struct
  q->read_index = 0;

  // reset window
  window_cf_reset( q );

  // return object
  return q;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// reset window object (initialize to zeros)
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void window_cf_reset( struct window_cf_s *_q )
{
  // reset read index
  _q->read_index = 0;

  // clear all allocated memory
  memset( _q->v, 0, ( _q->num_allocated )*sizeof( float complex) );
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// read window buffer contents
//  _q      : window object
//  _v      : output pointer (set to internal array)
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void window_cf_read( struct window_cf_s *_q, float complex **_v )
{
  // return pointer to buffer
  *_v = _q->v + _q->read_index;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// index single element in buffer at a particular index
//  _q      : window object
//  _i      : index of element to read
//  _v      : output value pointer
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void window_cf_index( struct window_cf_s *_q, unsigned int _i, float complex *_v )
{
  // return value at index
  *_v = _q->v[_q->read_index + _i];
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// push single element onto window buffer
//  _q      : window object
//  _v      : single input element
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void window_cf_push( struct window_cf_s *_q, float complex _v )
{
  // increment index
  _q->read_index++;

  // wrap around pointer
  _q->read_index &= _q->mask;

  // if pointer wraps around, copy excess memory
  if( _q->read_index == 0 )
    memmove( _q->v, _q->v + _q->n, ( _q->len - 1 )*sizeof( float complex) );

  // append value to end of buffer
  _q->v[_q->read_index + _q->len - 1] = _v;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// write array of elements onto window buffer
//  _q      : window object
//  _v      : input array of values to write
//  _n      : number of input values to write
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void window_cf_write( struct window_cf_s *_q, float complex *_v, unsigned int _n )
{
  // TODO make this more efficient
  unsigned int i;
  for( i = 0; i < _n; i++ )
    window_cf_push( _q, _v[i] );
}
