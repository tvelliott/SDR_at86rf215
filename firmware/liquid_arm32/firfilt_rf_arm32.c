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
// firfilt : finite impulse response (FIR) filter
//

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <float.h>
#include <complex.h>
#include <math.h>

#include "globals.h"
#include "firdes_arm32.h"
#include "firfilt_rf_arm32.h"
#include "msb_index_arm32.h"
#include "dotprod_rf_arm32.h"
#include "math_arm32.h"

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// create firfilt object
//  _h      :   coefficients (filter taps) [size: _n x 1]
//  _n      :   filter length
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
struct firfilt_rf_s * firfilt_rf_create(struct firfilt_rf_s *q,
												   float * _h,
                           unsigned int _n)
{
    q->h_len = _n;
    q->h = _h; 

    // initialize array for buffering
    q->w_len   = 1<<liquid_msb_index(q->h_len); // effectively 2^{floor(log2(len))+1}
    q->w_mask  = q->w_len - 1;
    q->w_index = 0;
    q->w       = (float *) malloc((q->w_len + q->h_len + 1)*sizeof(float));

		//printf("\r\nfirfilt_rf_create h_len %d, w_len %d", q->h_len, q->w_len);

    // load filter in reverse order
    unsigned int i;
    for (i=_n; i>0; i--)
        q->h[i-1] = _h[_n-i];

	  struct dotprod_rf_s *_dotprod_rf_s = malloc( sizeof(struct dotprod_rf_s) );

    // create dot product object
    q->dp = dotprod_rf_create(_dotprod_rf_s, q->h, q->h_len);

    // set default scaling
    q->scale = 1;

    // reset filter state (clear buffer)
    firfilt_rf_reset(q);

    return q;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// create filter using Kaiser-Bessel windowed sinc method
//  _n      : filter length, _n > 0
//  _fc     : cutoff frequency, 0 < _fc < 0.5
//  _As     : stop-band attenuation [dB], _As > 0
//  _mu     : fractional sample offset, -0.5 < _mu < 0.5
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
struct firfilt_rf_s * firfilt_rf_create_kaiser(struct firfilt_rf_s *q, unsigned int _n,
                                  float        _fc,
                                  float        _As,
                                  float        _mu)
{

		float *hf = malloc( _n * sizeof(float) );
    // compute temporary array for holding coefficients
    liquid_firdes_kaiser(_n, _fc, _As, _mu, hf);

		float *h = malloc( _n * sizeof(float) );
    // copy coefficients to type-specific array
    unsigned int i;
    for (i=0; i<_n; i++)
        h[i] = (float) hf[i];

		free(hf);

    // 
    return firfilt_rf_create(q, h, _n);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// create from square-root Nyquist prototype
//  _type   : filter type (e.g. LIQUID_RNYQUIST_RRC)
//  _k      : nominal samples/symbol, _k > 1
//  _m      : filter delay [symbols], _m > 0
//  _beta   : rolloff factor, 0 < beta <= 1
//  _mu     : fractional sample offset,-0.5 < _mu < 0.5
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
struct firfilt_rf_s * firfilt_rf_create_rnyquist(struct firfilt_rf_s *q, int          _type,
                                    unsigned int _k,
                                    unsigned int _m,
                                    float        _beta,
                                    float        _mu)
{
    // generate square-root Nyquist filter
    unsigned int h_len = 2*_k*_m + 1;
		//printf("\r\nfirfilt_rf_create_rnyquist: %d", h_len);
	
		float *hf = malloc( h_len * sizeof(float) );

    liquid_firdes_prototype(_type, _k, _m, _beta, _mu, hf);

		float *h = malloc( h_len * sizeof(float) );

    // copy coefficients to type-specific array (e.g. float complex)
    unsigned int i;
    for (i=0; i<h_len; i++)
        h[i] = hf[i];

		free(hf);

    // return filter object and return
    return firfilt_rf_create(q, h, h_len);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// create rectangular filter prototype
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
struct firfilt_rf_s * firfilt_rf_create_rect(struct firfilt_rf_s *q, unsigned int _n)
{
		float *hf = malloc( _n * sizeof(float) );

    // create float array coefficients
    unsigned int i;
    for (i=0; i<_n; i++)
        hf[i] = 1.0f;

		float *h = malloc( _n * sizeof(float) );

    // copy coefficients to type-specific array
    for (i=0; i<_n; i++)
        h[i] = (float) hf[i];

		free(hf);

    // return filter object and return
    return firfilt_rf_create(q, h, _n);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// reset internal state of filter object
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void firfilt_rf_reset(struct firfilt_rf_s * _q)
{
    unsigned int i;
    for (i=0; i<_q->w_len; i++)
        _q->w[i] = 0.0;
    _q->w_index = 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// set output scaling for filter
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void firfilt_rf_set_scale(struct firfilt_rf_s * _q,
                         float        _scale)
{
    _q->scale = _scale;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// get output scaling for filter
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void firfilt_rf_get_scale(struct firfilt_rf_s * _q,
                         float *      _scale)
{
    *_scale = _q->scale;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// push sample into filter object's internal buffer
//  _q      :   filter object
//  _x      :   input sample
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void firfilt_rf_push(struct firfilt_rf_s * _q,
                    float        _x)
{
    // increment index
    _q->w_index++;

    // wrap around pointer
    _q->w_index &= _q->w_mask;

    // if pointer wraps around, copy excess memory
    if (_q->w_index == 0)
        memmove(_q->w, _q->w + _q->w_len, (_q->h_len)*sizeof(float));

    // append value to end of buffer
    _q->w[_q->w_index + _q->h_len - 1] = _x;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// compute output sample (dot product between internal
// filter coefficients and internal buffer)
//  _q      :   filter object
//  _y      :   output sample pointer
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void firfilt_rf_execute(struct firfilt_rf_s * _q,
                       float *      _y)
{
    // read buffer (retrieve pointer to aligned memory array)
    float *r = _q->w + _q->w_index;

    // execute dot product
    dotprod_rf_execute(_q->dp, r, _y);

    // apply scaling factor
    *_y *= _q->scale;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// execute the filter on a block of input samples; the
// input and output buffers may be the same
//  _q      : filter object
//  _x      : pointer to input array [size: _n x 1]
//  _n      : number of input, output samples
//  _y      : pointer to output array [size: _n x 1]
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void firfilt_rf_execute_block(struct firfilt_rf_s *    _q,
                             float *         _x,
                             unsigned int _n,
                             float *         _y)
{
    unsigned int i;
    for (i=0; i<_n; i++) {
        // push sample into filter
        firfilt_rf_push(_q, _x[i]);

        // compute output sample
        firfilt_rf_execute(_q, &_y[i]);
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// get filter length
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
unsigned int firfilt_rf_get_length(struct firfilt_rf_s * _q)
{
    return _q->h_len;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// compute complex frequency response
//  _q      :   filter object
//  _fc     :   frequency
//  _H      :   output frequency response
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void firfilt_rf_freqresponse(struct firfilt_rf_s *       _q,
                            float           _fc,
                            float * _H)
{
    unsigned int i;
    float H = 0.0f;

    // compute dot product between coefficients and exp{ 2 pi fc {0..n-1} }
    for (i=0; i<_q->h_len; i++)
        H += _q->h[i] * cexpf(_Complex_I*2*M_PI*_fc*i);

    // apply scaling
    H *= _q->scale;

    // set return value
    *_H = H;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// compute group delay in samples
//  _q      :   filter object
//  _fc     :   frequency
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
float firfilt_rf_groupdelay(struct firfilt_rf_s * _q,
                           float     _fc)
{
    // copy coefficients to be in correct order
    float h[_q->h_len];
    unsigned int i;
    unsigned int n = _q->h_len;
    for (i=0; i<n; i++)
        h[i] = crealf(_q->h[n-i-1]);

    return fir_group_delay(h, n, _fc);
}
*/
