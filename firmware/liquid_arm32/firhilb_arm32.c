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
// firhilb.c
//
// finite impulse response (FIR) Hilbert transform
//

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <complex.h>

#include "globals.h"
#include "firdes_arm32.h"
#include "firhilb_arm32.h"
#include "window_rf_arm32.h"
#include "dotprod_rf_arm32.h"


////////////////////////////////////////////////////////////////////////////////////////////////////
// create firhilb object
//  _m      :   filter semi-length (delay: 2*m+1)
//  _As     :   stop-band attenuation [dB]
////////////////////////////////////////////////////////////////////////////////////////////////////
struct firhilb_s *  firhilb_create(unsigned int _m,
                           float        _As)
{
    // allocate memory for main object
    struct firhilb_s *  q = (struct firhilb_s * ) malloc(sizeof(struct firhilb_s));
    q->m  = _m;         // filter semi-length
    q->As = fabsf(_As); // stop-band attenuation

    // set filter length and allocate memory for coefficients
    q->h_len = 4*(q->m) + 1;
    q->h     = (float *)         malloc((q->h_len)*sizeof(float));
    q->hc    = (float complex *) malloc((q->h_len)*sizeof(float complex));

    // allocate memory for quadrature filter component
    q->hq_len = 2*(q->m);
    q->hq     = (float *) malloc((q->hq_len)*sizeof(float));

    // compute filter coefficients for half-band filter
    liquid_firdes_kaiser(q->h_len, 0.25f, q->As, 0.0f, q->h);

    // alternate sign of non-zero elements
    unsigned int i;
    for (i=0; i<q->h_len; i++) {
        float t = (float)i - (float)(q->h_len-1)/2.0f;
        q->hc[i] = q->h[i] * cexpf(_Complex_I*0.5f*M_PI*t);
        q->h[i]  = cimagf(q->hc[i]);
    }

    // resample, reverse direction
    unsigned int j=0;
    for (i=1; i<q->h_len; i+=2) {
			q->hq[j++] = q->h[q->h_len - i - 1];
		}

	struct window_rf_s *w1_i = malloc( sizeof(struct window_rf_s) );
	struct window_rf_s *w2_i = malloc( sizeof(struct window_rf_s) );

    // create windows for upper and lower polyphase filter branches
    q->w1 = window_rf_create(w1_i, 2*(q->m));
    q->w0 = window_rf_create(w2_i, 2*(q->m));

	struct dotprod_rf_s *dp1 = malloc( sizeof(struct dotprod_rf_s) );

    // create internal dot product object
    q->dpq = dotprod_rf_create(dp1, q->hq, q->hq_len);

    // reset internal state and return object
    firhilb_reset(q);
    return q;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// reset firhilb object internal state
////////////////////////////////////////////////////////////////////////////////////////////////////
void firhilb_reset(struct firhilb_s *  _q)
{
    // clear window buffers
    window_rf_reset(_q->w0);
    window_rf_reset(_q->w1);

    // reset toggle flag
    _q->toggle = 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// execute Hilbert transform (real to complex)
//  _q      :   firhilb object
//  _x      :   real-valued input sample
//  _y      :   complex-valued output sample
////////////////////////////////////////////////////////////////////////////////////////////////////
void firhilb_r2c_execute(struct firhilb_s *_q,
                           float           _x,
                           float complex *_y)
{
    float * r;  // buffer read pointer
    float yi;   // in-phase component
    float yq;   // quadrature component

    if ( _q->toggle == 0 ) {
        // push sample into upper branch
        window_rf_push(_q->w0, _x);

        // upper branch (delay)
        window_rf_index(_q->w0, _q->m-1, &yi);

        // lower branch (filter)
        window_rf_read(_q->w1, &r);
        
        // execute dotprod
        dotprod_rf_execute(_q->dpq, r, &yq);
    } else {
        // push sample into lower branch
        window_rf_push(_q->w1, _x);

        // upper branch (delay)
        window_rf_index(_q->w1, _q->m-1, &yi);

        // lower branch (filter)
        window_rf_read(_q->w0, &r);

        // execute dotprod
        dotprod_rf_execute(_q->dpq, r, &yq);
    }

    // toggle flag
    _q->toggle = 1 - _q->toggle;

    // set return value
    *_y = yi + _Complex_I * yq;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// execute Hilbert transform (complex to real)
//  _q      :   firhilb object
//  _y      :   complex-valued input sample
//  _x      :   real-valued output sample
////////////////////////////////////////////////////////////////////////////////////////////////////
void firhilb_c2r_execute(struct firhilb_s *_q,
                           float complex _x,
                           float *_y)
{
    *_y = crealf(_x);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// execute Hilbert transform decimator (real to complex)
//  _q      :   firhilb object
//  _x      :   real-valued input array [size: 2 x 1]
//  _y      :   complex-valued output sample
////////////////////////////////////////////////////////////////////////////////////////////////////
void firhilb_decim_execute(struct firhilb_s *_q,
                             float *_x,
                             float complex *_y)
{
    float * r;  // buffer read pointer
    float yi;   // in-phase component
    float yq;   // quadrature component

    // compute quadrature component (filter branch)
    window_rf_push(_q->w1, _x[0]);
    window_rf_read(_q->w1, &r);
    dotprod_rf_execute(_q->dpq, r, &yq);

    window_rf_push(_q->w0, _x[1]);
    window_rf_index(_q->w0, _q->m-1, &yi);

    // set return value
    *_y = yi + _Complex_I * yq;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// execute Hilbert transform decimator (real to complex) on
// a block of samples
//  _q      :   Hilbert transform object
//  _x      :   real-valued input array [size: 2*_n x 1]
//  _n      :   number of *output* samples
//  _y      :   complex-valued output array [size: _n x 1]
////////////////////////////////////////////////////////////////////////////////////////////////////
void firhilb_decim_execute_block(struct firhilb_s *_q,
                                   float *_x,
                                   unsigned int _n,
                                   float complex *_y)
{
    unsigned int i;

    for (i=0; i<_n; i++)
        firhilb_decim_execute(_q, &_x[2*i], &_y[i]);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// execute Hilbert transform interpolator (complex to real)
//  _q      :   firhilb object
//  _y      :   complex-valued input sample
//  _x      :   real-valued output array [size: 2 x 1]
////////////////////////////////////////////////////////////////////////////////////////////////////
void firhilb_interp_execute(struct firhilb_s *_q,
                              float complex _x,
                              float *_y)
{
    float * r;  // buffer read pointer

    // TODO macro for crealf, cimagf?
    
    window_rf_push(_q->w0, cimagf(_x));
    window_rf_index(_q->w0, _q->m-1, &_y[0]);

    // compute second branch (filter)
    window_rf_push(_q->w1, crealf(_x));
    window_rf_read(_q->w1, &r);
    dotprod_rf_execute(_q->dpq, r, &_y[1]);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// execute Hilbert transform interpolator (complex to real)
// on a block of samples
//  _q      :   Hilbert transform object
//  _x      :   complex-valued input array [size: _n x 1]
//  _n      :   number of *input* samples
//  _y      :   real-valued output array [size: 2*_n x 1]
////////////////////////////////////////////////////////////////////////////////////////////////////
void firhilb_interp_execute_block(struct firhilb_s *_q,
                                    float complex *_x,
                                    unsigned int _n,
                                    float *          _y)
{
    unsigned int i;

    for (i=0; i<_n; i++)
        firhilb_interp_execute(_q, _x[i], &_y[2*i]);
}
