/*
//
//t.elliott converted from original liquid-dsp code to something that would be usable on the STM32H7 series
//
 * Copyright (c) 2007 - 2017 Joseph Gaeddert
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
// Infinite impulse response filter (second-order section)
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <complex.h>

#include "iirfiltsos_rrrf_arm32.h"

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// create iirfiltsos object
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
struct iirfiltsos_rrrf_s *iirfiltsos_rrrf_create( struct iirfiltsos_rrrf_s *q, float *_b, float *_a )
{
  // set the internal coefficients
  iirfiltsos_rrrf_set_coefficients( q, _b, _a );

  // clear filter state
  iirfiltsos_rrrf_reset( q );

  return q;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// set internal filter coefficients
// NOTE : this does not reset the internal state of the filter and
//        could result in instability if executed on existing filter!
// explicitly set 2nd-order IIR filter coefficients
//  _q      : iirfiltsos object
//  _b      : feed-forward coefficients [size: _3 x 1]
//  _a      : feed-back coefficients    [size: _3 x 1]
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
void iirfiltsos_rrrf_set_coefficients( struct iirfiltsos_rrrf_s *_q, float *_b, float *_a )
{
  // retain a0 coefficient for normalization
  float a0 = _a[0];

  // copy feed-forward coefficients (numerator)
  _q->b[0] = _b[0] / a0;
  _q->b[1] = _b[1] / a0;
  _q->b[2] = _b[2] / a0;

  // copy feed-back coefficients (denominator)
  _q->a[0] = _a[0] / a0;  // unity
  _q->a[1] = _a[1] / a0;
  _q->a[2] = _a[2] / a0;

}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// clear/reset iirfiltsos object internals
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
void iirfiltsos_rrrf_reset( struct iirfiltsos_rrrf_s *_q )
{
  // set to zero
  _q->v[0] = 0;
  _q->v[1] = 0;
  _q->v[2] = 0;

  _q->x[0] = 0;
  _q->x[1] = 0;
  _q->x[2] = 0;

  _q->y[0] = 0;
  _q->y[1] = 0;
  _q->y[2] = 0;

}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// compute filter output
//  _q      : iirfiltsos object
//  _x      : input sample
//  _y      : output sample pointer
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
void iirfiltsos_rrrf_execute( struct iirfiltsos_rrrf_s *_q, float _x, float *_y )
{
  // execute type-specific code
  iirfiltsos_rrrf_execute_df2( _q, _x, _y );
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// compute filter output, direct form I method
//  _q      : iirfiltsos object
//  _x      : input sample
//  _y      : output sample pointer
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
void iirfiltsos_rrrf_execute_df1( struct iirfiltsos_rrrf_s *_q, float _x, float *_y )
{
  // advance buffer x
  _q->x[2] = _q->x[1];
  _q->x[1] = _q->x[0];
  _q->x[0] = _x;

  // advance buffer y
  _q->y[2] = _q->y[1];
  _q->y[1] = _q->y[0];

  // compute new v
  float v = _q->x[0] * _q->b[0] +
            _q->x[1] * _q->b[1] +
            _q->x[2] * _q->b[2];

  // compute new y[0]
  _q->y[0] = v -
             _q->y[1] * _q->a[1] -
             _q->y[2] * _q->a[2];

  // set output
  *_y = _q->y[0];
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// compute filter output, direct form II method
//  _q      : iirfiltsos object
//  _x      : input sample
//  _y      : output sample pointer
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
void iirfiltsos_rrrf_execute_df2( struct iirfiltsos_rrrf_s *_q, float _x, float *_y )
{
  // advance buffer
  _q->v[2] = _q->v[1];
  _q->v[1] = _q->v[0];

  // compute new v[0]
  _q->v[0] = _x -
             _q->a[1] * _q->v[1] -
             _q->a[2] * _q->v[2];

  // compute output _y
  *_y = _q->b[0] * _q->v[0] +
        _q->b[1] * _q->v[1] +
        _q->b[2] * _q->v[2];
}

/*
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// compute group delay in samples
//  _q      :   filter object
//  _fc     :   frequency
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
float iirfiltsos_rrrf_groupdelay( struct iirfiltsos_rrrf_s *_q, float _fc )
{
  // copy coefficients
  float b[3];
  float a[3];
  unsigned int i;
  for( i = 0; i < 3; i++ ) {
    b[i] = crealf( _q->b[i] );
    a[i] = crealf( _q->a[i] );
  }
  return iir_group_delay( b, 3, a, 3, _fc ) + 2.0;
}
*/
