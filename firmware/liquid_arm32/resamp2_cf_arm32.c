/*
//
//t.elliott converted from original liquid-dsp code to something that would be usable on the STM32H7 series
//
 * Copyright (c) 2007 - 2018 Joseph Gaeddert
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
// Halfband resampler (interpolator/decimator)
//

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <complex.h>

#include "globals.h"
#include "window_functions_arm32.h"
#include "firdes_arm32.h"
#include "math_arm32.h"
#include "resamp2_cf_arm32.h"

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// create a resamp2 object
//  _m      :   filter semi-length (effective length: 4*_m+1)
//  _f0     :   center frequency of half-band filter
//  _As     :   stop-band attenuation [dB], _As > 0
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
struct resamp2_s *  resamp2_create(struct resamp2_s *q, unsigned int _m,
                           float        _f0,
                           float        _As)
{
    //q = (struct resamp2_s * ) malloc(sizeof(struct resamp2_s));
    q->m  = _m;
    q->f0 = _f0;
    q->As = _As;

    // change filter length as necessary
    q->h_len = 4*(q->m) + 1;
    q->h = (float complex *) malloc((q->h_len)*sizeof(float complex));

    q->h1_len = 2*(q->m);
    q->h1 = (float complex *) malloc((q->h1_len)*sizeof(float complex));

    // design filter prototype
    unsigned int i;
    float t, h1, h2;
    float complex h3;
    float beta = kaiser_beta_As(q->As);
    for (i=0; i<q->h_len; i++) {
        t = (float)i - (float)(q->h_len-1)/2.0f;
        h1 = sincf(t/2.0f);
        h2 = kaiser(i,q->h_len,beta,0);
        h3 = cosf(2.0f*M_PI*t*q->f0) + _Complex_I*sinf(2.0f*M_PI*t*q->f0);
        q->h[i] = h1*h2*h3;
    }

    // resample, alternate sign, [reverse direction]
    unsigned int j=0;
    for (i=1; i<q->h_len; i+=2)
        q->h1[j++] = q->h[q->h_len - i - 1];

    struct dotprod_cf_s *dprod = malloc( sizeof(struct dotprod_cf_s) );
    // create dotprod object
    q->dp = dotprod_cf_create(dprod, q->h1, 2*q->m);

    struct window_cf_s *win0 = malloc( sizeof(struct window_cf_s) );
    struct window_cf_s *win1 = malloc( sizeof(struct window_cf_s) );
    // create window buffers
    q->w0 = window_cf_create(win0, 2*(q->m));
    q->w1 = window_cf_create(win1, 2*(q->m));

    resamp2_reset(q);

    return q;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// clear internal buffer
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void resamp2_reset(struct resamp2_s *  _q)
{
    window_cf_reset(_q->w0);
    window_cf_reset(_q->w1);

    _q->toggle = 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// get filter delay (samples)
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
unsigned int resamp2_get_delay(struct resamp2_s *  _q)
{
    return 2*_q->m - 1;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// execute resamp2 as half-band filter
//  _q      :   resamp2 object
//  _x      :   input sample
//  _y0     :   output sample pointer (low frequency)
//  _y1     :   output sample pointer (high frequency)
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void resamp2_filter_execute(struct resamp2_s *  _q,
                              float complex        _x,
                              float complex *      _y0,
                              float complex *      _y1)
{
    float complex * r;     // buffer read pointer
    float complex yi;      // delay branch
    float complex yq;      // filter branch

    if ( _q->toggle == 0 ) {
        // push sample into upper branch
        window_cf_push(_q->w0, _x);

        // upper branch (delay)
        window_cf_index(_q->w0, _q->m-1, &yi);

        // lower branch (filter)
        window_cf_read(_q->w1, &r);
        dotprod_cf_execute(_q->dp, r, &yq);
    } else {
        // push sample into lower branch
        window_cf_push(_q->w1, _x);

        // upper branch (delay)
        window_cf_index(_q->w1, _q->m-1, &yi);

        // lower branch (filter)
        window_cf_read(_q->w0, &r);
        dotprod_cf_execute(_q->dp, r, &yq);
    }

    // toggle flag
    _q->toggle = 1 - _q->toggle;

    // set return values, normalizing gain
    *_y0 = 0.5f*(yi + yq);  // lower band
    *_y1 = 0.5f*(yi - yq);  // upper band
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// execute analysis half-band filterbank
//  _q      :   resamp2 object
//  _x      :   input array [size: 2 x 1]
//  _y      :   output array [size: 2 x 1]
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void resamp2_analyzer_execute(struct resamp2_s *  _q,
                                float complex *      _x,
                                float complex *      _y)
{
    float complex * r;     // buffer read pointer
    float complex y0;      // delay branch
    float complex y1;      // filter branch

    // compute filter branch
    window_cf_push(_q->w1, 0.5*_x[0]);
    window_cf_read(_q->w1, &r);
    dotprod_cf_execute(_q->dp, r, &y1);

    // compute delay branch
    window_cf_push(_q->w0, 0.5*_x[1]);
    window_cf_index(_q->w0, _q->m-1, &y0);

    // set return value
    _y[0] = y1 + y0;
    _y[1] = y1 - y0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// execute synthesis half-band filterbank
//  _q      :   resamp2 object
//  _x      :   input array [size: 2 x 1]
//  _y      :   output array [size: 2 x 1]
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void resamp2_synthesizer_execute(struct resamp2_s *  _q,
                                   float complex *      _x,
                                   float complex *      _y)
{
    float complex * r;                 // buffer read pointer
    float complex x0 = _x[0] + _x[1];  // delay branch input
    float complex x1 = _x[0] - _x[1];  // filter branch input

    // compute delay branch
    window_cf_push(_q->w0, x0);
    window_cf_index(_q->w0, _q->m-1, &_y[0]);

    // compute second branch (filter)
    window_cf_push(_q->w1, x1);
    window_cf_read(_q->w1, &r);
    dotprod_cf_execute(_q->dp, r, &_y[1]);
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// execute half-band decimation
//  _q      :   resamp2 object
//  _x      :   input array [size: 2 x 1]
//  _y      :   output sample pointer
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void resamp2_decim_execute(struct resamp2_s *  _q,
                             float complex *      _x,
                             float complex *      _y)
{
    float complex * r;     // buffer read pointer
    float complex y0;      // delay branch
    float complex y1;      // filter branch

    // compute filter branch
    window_cf_push(_q->w1, _x[0]);
    window_cf_read(_q->w1, &r);
    dotprod_cf_execute(_q->dp, r, &y1);

    // compute delay branch
    window_cf_push(_q->w0, _x[1]);
    window_cf_index(_q->w0, _q->m-1, &y0);

    // set return value
    *_y = y0 + y1;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// execute half-band interpolation
//  _q      :   resamp2 object
//  _x      :   input sample
//  _y      :   output array [size: 2 x 1]
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void resamp2_interp_execute(struct resamp2_s *  _q,
                              float complex        _x,
                              float complex *      _y)
{
    float complex * r;  // buffer read pointer

    // compute delay branch
    window_cf_push(_q->w0, _x);
    window_cf_index(_q->w0, _q->m-1, &_y[0]);

    // compute second branch (filter)
    window_cf_push(_q->w1, _x);
    window_cf_read(_q->w1, &r);
    dotprod_cf_execute(_q->dp, r, &_y[1]);
}

