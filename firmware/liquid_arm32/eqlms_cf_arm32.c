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
// Least mean-squares (LMS) equalizer
//

#include <math.h>
#include <complex.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "firdes_arm32.h"
#include "window_cf_arm32.h"
#include "wdelay_cf_arm32.h"
#include "eqlms_cf_arm32.h"


///////////////////////////////////////////////////////////////////////////////////////////////////////////
// create least mean-squares (LMS) equalizer object
//  _h      :   initial coefficients [size: _h_len x 1], default if NULL
//  _p      :   equalizer length (number of taps)
///////////////////////////////////////////////////////////////////////////////////////////////////////////
struct eqlms_cf_s *  eqlms_cf_create(float complex *_h,
                       unsigned int _h_len)
{
    struct eqlms_cf_s *  q = (struct eqlms_cf_s * ) malloc(sizeof(struct eqlms_cf_s));

    // set filter order, other params
    q->h_len = _h_len;
    q->mu    = 0.5f;

    q->h0 = (float complex *) malloc((q->h_len)*sizeof(float complex));
    q->w0 = (float complex *) malloc((q->h_len)*sizeof(float complex));
    q->w1 = (float complex *) malloc((q->h_len)*sizeof(float complex));

		struct window_cf_s *_window_cf_s;
		_window_cf_s = malloc( sizeof(struct window_cf_s) );

    q->buffer = window_cf_create(_window_cf_s, q->h_len);

    q->x2     = wdelay_cf_create(q->h_len);

    // copy coefficients (if not NULL)
    if (_h == NULL) {
        // initial coefficients with delta at first index
        unsigned int i;
        for (i=0; i<q->h_len; i++)
            q->h0[i] = (i==0) ? 1.0 : 0.0;
    } else {
        // copy user-defined initial coefficients
        memmove(q->h0, _h, (q->h_len)*sizeof(float complex));
    }

    // reset equalizer object
    eqlms_cf_reset(q);

    // return main object
    return q;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
// create square-root Nyquist interpolator
//  _type   :   filter type (e.g. LIQUID_RNYQUIST_RRC)
//  _k      :   samples/symbol _k > 1
//  _m      :   filter delay (symbols), _m > 0
//  _beta   :   excess bandwidth factor, 0 < _beta < 1
//  _dt     :   fractional sample delay, 0 <= _dt < 1
///////////////////////////////////////////////////////////////////////////////////////////////////////////
struct eqlms_cf_s *  eqlms_cf_create_rnyquist(int _type,
                                unsigned int _k,
                                unsigned int _m,
                                float        _beta,
                                float        _dt)
{
    // generate square-root Nyquist filter
    unsigned int h_len = 2*_k*_m + 1;
    float h[h_len];
    liquid_firdes_prototype(_type,_k,_m,_beta,_dt,h);

    // copy coefficients to type-specific array (e.g. float complex)
    // and scale by samples/symbol
    unsigned int i;
    float complex hc[h_len];
    for (i=0; i<h_len; i++)
        hc[i] = h[i] / (float)_k;

    // return equalizer object
    return eqlms_cf_create(hc, h_len);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
// create LMS EQ initialized with low-pass filter
//  _h_len  : filter length
//  _fc     : filter cut-off, _fc in (0,0.5]
///////////////////////////////////////////////////////////////////////////////////////////////////////////
struct eqlms_cf_s *  eqlms_cf_create_lowpass(unsigned int _h_len,
                               float        _fc)
{
    // generate low-pass filter prototype
    float h[_h_len];
    liquid_firdes_kaiser(_h_len, _fc, 40.0f, 0.0f, h);

    // copy coefficients to type-specific array (e.g. float complex)
    unsigned int i;
    float complex hc[_h_len];
    for (i=0; i<_h_len; i++)
        hc[i] = h[i];

    // return equalizer object
    return eqlms_cf_create(hc, _h_len);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
// reset equalizer
///////////////////////////////////////////////////////////////////////////////////////////////////////////
void eqlms_cf_reset(struct eqlms_cf_s *_q)
{
    // copy default coefficients
    memmove(_q->w0, _q->h0, (_q->h_len)*sizeof(float complex));

    window_cf_reset(_q->buffer);
    wdelay_cf_reset(_q->x2);

    // reset input count
    _q->count = 0;
    _q->buf_full = 0;

    // reset squared magnitude sum
    _q->x2_sum = 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
// get learning rate of equalizer
///////////////////////////////////////////////////////////////////////////////////////////////////////////
float eqlms_cf_get_bw(struct eqlms_cf_s *_q)
{
    return _q->mu;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
// set learning rate of equalizer
//  _q      :   equalizer object
//  _mu     :   LMS learning rate (should be near 0), 0 < _mu < 1
///////////////////////////////////////////////////////////////////////////////////////////////////////////
void eqlms_cf_set_bw(struct eqlms_cf_s *_q,
                    float   _mu)
{
    _q->mu = _mu;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
// push sample into equalizer internal buffer
//  _q      :   equalizer object
//  _x      :   received sample
///////////////////////////////////////////////////////////////////////////////////////////////////////////
void eqlms_cf_push(struct eqlms_cf_s *_q,
                  float complex _x)
{
    // push value into buffer
    window_cf_push(_q->buffer, _x);

    // update sum{|x|^2}
    eqlms_cf_update_sumsq(_q, _x);

    // increment count
    _q->count++;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
// push sample into equalizer internal buffer as block
//  _q      :   equalizer object
//  _x      :   input sample array
//  _n      :   input sample array length
///////////////////////////////////////////////////////////////////////////////////////////////////////////
void eqlms_cf_push_block(struct eqlms_cf_s *_q,
                        float complex *_x,
                        unsigned int _n)
{
    unsigned int i;
    for (i=0; i<_n; i++)
        eqlms_cf_push(_q, _x[i]);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
// execute internal dot product
//  _q      :   equalizer object
//  _y      :   output sample
///////////////////////////////////////////////////////////////////////////////////////////////////////////
void eqlms_cf_execute(struct eqlms_cf_s *_q,
                     float complex *_y)
{
    float complex y = 0;    // temporary accumulator
    float complex *r;      // read buffer
    window_cf_read(_q->buffer, &r);

    // compute conjugate vector dot product
    //DOTPROD(_run)(_q->w0, r, _q->h_len, &y);
    unsigned int i;
    for (i=0; i<_q->h_len; i++) {
        float complex sum = conj(_q->w0[i])*r[i];
        y += sum;
    }

    // set output
    *_y = y;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
// execute equalizer with block of samples using constant
// modulus algorithm, operating on a decimation rate of _k
// samples.
//  _q      :   equalizer object
//  _k      :   down-sampling rate
//  _x      :   input sample array [size: _n x 1]
//  _n      :   input sample array length
//  _y      :   output sample array [size: _n x 1]
///////////////////////////////////////////////////////////////////////////////////////////////////////////
void eqlms_cf_execute_block(struct eqlms_cf_s *_q,
                           unsigned int _k,
                           float complex *_x,
                           unsigned int _n,
                           float complex *_y)
{
    unsigned int i;
    float complex d_hat;
    for (i=0; i<_n; i++) {
        // push input sample
        eqlms_cf_push(_q, _x[i]);

        // compute output sample
        eqlms_cf_execute(_q, &d_hat);

        // store result in output
        _y[i] = d_hat;

        // decimate by _k
        if ( ((_q->count+_k-1) % _k) == 0 ) {
            // update equalizer independent of the signal: estimate error
            // assuming constant modulus signal
            eqlms_cf_step_blind(_q, d_hat);
        }
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
// step through one cycle of equalizer training
//  _q      :   equalizer object
//  _d      :   desired output
//  _d_hat  :   filtered output
///////////////////////////////////////////////////////////////////////////////////////////////////////////
void eqlms_cf_step(struct eqlms_cf_s *_q,
                  float complex       _d,
                  float complex       _d_hat)
{
    // check count; only run step when buffer is full
    if (!_q->buf_full) {
        if (_q->count < _q->h_len)
            return;
        else
            _q->buf_full = 1;
    }

    unsigned int i;

    // compute error (a priori)
    float complex alpha = _d - _d_hat;

    // read buffer
    float complex *r;      // read buffer
    window_cf_read(_q->buffer, &r);

    // update weighting vector
    // w[n+1] = w[n] + mu*conj(d-d_hat)*x[n]/(x[n]' * conj(x[n]))
    for (i=0; i<_q->h_len; i++)
        _q->w1[i] = _q->w0[i] + (_q->mu)*conj(alpha)*r[i]/_q->x2_sum;

    // copy old values
    memmove(_q->w0, _q->w1, _q->h_len*sizeof(float complex));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
// step through one cycle of equalizer training
//  _q      :   equalizer object
//  _d_hat  :   filtered output
///////////////////////////////////////////////////////////////////////////////////////////////////////////
void eqlms_cf_step_blind(struct eqlms_cf_s *_q,
                        float complex       _d_hat)
{
    // update equalizer using constant modulus method
    float complex d = _d_hat / cabsf(_d_hat);	//complex version
    eqlms_cf_step(_q, d, _d_hat);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
// retrieve internal filter coefficients
///////////////////////////////////////////////////////////////////////////////////////////////////////////
void eqlms_cf_get_weights(struct eqlms_cf_s *_q, float complex *_w)
{
    // copy output weight vector
    unsigned int i;
    for (i=0; i<_q->h_len; i++)
        _w[i] = conj(_q->w0[_q->h_len-i-1]);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
// train equalizer object
//  _q      :   equalizer object
//  _w      :   initial weights / output weights
//  _x      :   received sample vector
//  _d      :   desired output vector
//  _n      :   vector length
///////////////////////////////////////////////////////////////////////////////////////////////////////////
void eqlms_cf_train(struct eqlms_cf_s *_q,
                   float complex *_w,
                   float complex *_x,
                   float complex *_d,
                   unsigned int _n)
{
    unsigned int i;
    unsigned int p=_q->h_len;

    // reset equalizer state
    eqlms_cf_reset(_q);

    // copy initial weights into buffer
    for (i=0; i<p; i++)
        _q->w0[i] = _w[p - i - 1];

    float complex d_hat;
    for (i=0; i<_n; i++) {
        // push sample into internal buffer
        eqlms_cf_push(_q, _x[i]);

        // execute vector dot product
        eqlms_cf_execute(_q, &d_hat);

        // step through training cycle
        eqlms_cf_step(_q, _d[i], d_hat);
    }

    // copy output weight vector
    eqlms_cf_get_weights(_q, _w);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
// 
// internal methods
//
// update sum{|x|^2}
///////////////////////////////////////////////////////////////////////////////////////////////////////////
void eqlms_cf_update_sumsq(struct eqlms_cf_s *_q, float complex _x)
{
    // update estimate of signal magnitude squared
    // |x[n-1]|^2 (input sample)
    float x2_n = crealf(_x * conjf(_x));

    // |x[0]  |^2 (oldest sample)
    float complex x2_0;

    // read oldest sample
    wdelay_cf_read(_q->x2, &x2_0);

    // push newest sample
    wdelay_cf_push(_q->x2, x2_n);

    // update sum( |x|^2 ) of last 'n' input samples
    _q->x2_sum = _q->x2_sum + x2_n - x2_0;
}
