/*
//
//t.elliott converted from original liquid-dsp code to something that would be usable on the STM32H7 series
//
 * Copyright (c) 2007 - 2015 Joseph Gaeddert
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
// Generic dot product
//

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <complex.h>

#define ARM_MATH_CM7 1
#include "arm_common_tables.h"
#include "arm_math.h"

#include "dotprod_cf_arm32.h"

	 /*
//////////////////////////////////////////////////////////////////////////////////////////////////////////
// basic dot product
//  _h      :   coefficients array [size: 1 x _n]
//  _x      :   input array [size: 1 x _n]
//  _n      :   input lengths
//  _y      :   output dot product
//////////////////////////////////////////////////////////////////////////////////////////////////////////
void dotprod_cf_run( float complex *_h, float complex *_x, unsigned int _n, float complex *_y )
{
  // initialize accumulator
  float complex r = 0;

  unsigned int i;
  for( i = 0; i < _n; i++ )
    r += _h[i] * _x[i];

  // return result
  *_y = r;
}
*/

//////////////////////////////////////////////////////////////////////////////////////////////////////////
// basic dotproduct, unrolling loop
//  _h      :   coefficients array [size: 1 x _n]
//  _x      :   input array [size: 1 x _n]
//  _n      :   input lengths
//  _y      :   output dot product
//////////////////////////////////////////////////////////////////////////////////////////////////////////
void dotprod_cf_run4( float complex *_h, float complex *_x, unsigned int _n, float complex *_y )
{
  // initialize accumulator
  float complex r = 0;

  // t = 4*(floor(_n/4))
  unsigned int t = ( _n >> 2 ) << 2;

  // compute dotprod in groups of 4
  unsigned int i;
  for( i = 0; i < t; i += 4 ) {
    r += _h[i]   * _x[i];
    r += _h[i + 1] * _x[i + 1];
    r += _h[i + 2] * _x[i + 2];
    r += _h[i + 3] * _x[i + 3];
  }

  // clean up remaining
  for( ; i < _n; i++ )
    r += _h[i] * _x[i];

  // return result
  *_y = r;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
// create structured dot product object
//  _h      :   coefficients array [size: 1 x _n]
//  _n      :   dot product length
//////////////////////////////////////////////////////////////////////////////////////////////////////////
struct dotprod_cf_s *dotprod_cf_create( struct dotprod_cf_s *q, float complex *_h, unsigned int _n )
{
  q->n = _n;

  // allocate memory for coefficients
  //q->h = _h;
	q->h = malloc( _n * sizeof(float complex) );	

  int i;
  for( i = 0; i < _n; i++ ) {
    q->h[i] = _h[i];
    //printf("\r\ndotprod(%08x)[%d] = %3.4f", (unsigned int) q, i, q->h[i] );
  }

  // return object
  return q;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
// execute structured dot product
//  _q      :   dot product object
//  _x      :   input array [size: 1 x _n]
//  _y      :   output dot product
//////////////////////////////////////////////////////////////////////////////////////////////////////////
void dotprod_cf_execute( struct dotprod_cf_s *_q, float complex *_x, float complex *_y )
{
	#if 0
  // initialize accumulator
  float complex r = 0;

  // t = 4*(floor(_n/4))
  unsigned int t = ( _q->n >> 2 ) << 2;

  // compute dotprod in groups of 4
  unsigned int i;
  for( i = 0; i < t; i += 4 ) {
    r += _q->h[i]   * _x[i];
    r += _q->h[i + 1] * _x[i + 1];
    r += _q->h[i + 2] * _x[i + 2];
    r += _q->h[i + 3] * _x[i + 3];
  }

  // clean up remaining
  for( ; i < _q->n; i++ )
    r += _q->h[i] * _x[i];

  // return result
  *_y = r;
	#endif

	#if 1
		float *result = (float *) _y; 
		arm_cmplx_dot_prod_f32( (float32_t *) _q->h, (float32_t *) _x, _q->n, &result[0], &result[1]); 
	#endif
}
