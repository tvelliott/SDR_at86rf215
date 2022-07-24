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
// Windowing functions
//
// References:
//  [Kaiser:1980] James F. Kaiser and Ronald W. Schafer, "On
//      the Use of I0-Sinh Window for Spectrum Analysis,"
//      IEEE Transactions on Acoustics, Speech, and Signal
//      Processing, vol. ASSP-28, no. 1, pp. 105--107,
//      February, 1980.
//  [harris:1978] frederic j. harris, "On the Use of Windows for Harmonic
//      Analysis with the Discrete Fourier Transform," Proceedings of the
//      IEEE, vol. 66, no. 1, January, 1978.
//  [Nuttall:1981] Albert H. Nuttall, "Some Windows with Very Good Sidelobe
//      Behavior,"  IEEE Transactions on Acoustics, Speech, and Signal
//      Processing, vol. ASSP-29, no. 1, pp. 84-91, February, 1981.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "globals.h"
#include "math_arm32.h"
#include "window_functions_arm32.h"

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Kaiser-Bessel derived window
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
float liquid_kbd(unsigned int _n,
                 unsigned int _N,
                 float _beta)
{
    // TODO add reference

    unsigned int M = _N / 2;
    if (_n >= M)
        return liquid_kbd(_N-_n-1,_N,_beta);

    float w0 = 0.0f;
    float w1 = 0.0f;
    float w;
    unsigned int i;
    for (i=0; i<=M; i++) {
        // compute Kaiser window
        w = kaiser(i,M+1,_beta,0.0f);

        // accumulate window sums
        w1 += w;
        if (i <= _n) w0 += w;
    }
    //printf("%12.8f / %12.8f = %12.8f\n", w0, w1, w0/w1);

    return sqrtf(w0 / w1);
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Kaiser-Bessel derived window (full window function)
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void liquid_kbd_window(unsigned int _n,
                       float _beta,
                       float * _w)
{
    unsigned int i;
    // TODO add reference

    // compute half length
    unsigned int M = _n / 2;

    // generate regular Kaiser window, length M+1
    float w_kaiser[M+1];
    for (i=0; i<=M; i++)
        w_kaiser[i] = kaiser(i,M+1,_beta,0.0f);

    // compute sum(wk[])
    float w_sum = 0.0f;
    for (i=0; i<=M; i++)
        w_sum += w_kaiser[i];

    // accumulate window
    float w_acc = 0.0f;
    for (i=0; i<M; i++) {
        w_acc += w_kaiser[i];
        _w[i] = sqrtf(w_acc / w_sum);
    }

    // window has even symmetry; flip around index M
    for (i=0; i<M; i++)
        _w[_n-i-1] = _w[i];
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Kaiser window [Kaiser:1980]
//  _n      :   sample index
//  _N      :   window length (samples)
//  _beta   :   window taper parameter
//  _mu     :   fractional sample offset
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
float kaiser(unsigned int _n,
             unsigned int _N,
             float        _beta,
             float        _mu)
{

    float t = (float)_n - (float)(_N-1)/2 + _mu;
    float r = 2.0f*t/(float)(_N);
    float a = liquid_besseli0f(_beta*sqrtf(1-r*r));
    float b = liquid_besseli0f(_beta);
    return a / b;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Hamming window [Nuttall:1981]
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
float hamming(unsigned int _n,
              unsigned int _N)
{
    return 0.53836 - 0.46164*cosf( (2*M_PI*(float)_n) / ((float)(_N-1)) );
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Hann window
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
float hann(unsigned int _n,
           unsigned int _N)
{
    // TODO test this function
    // TODO add reference
    return 0.5f - 0.5f*cosf( (2*M_PI*(float)_n) / ((float)(_N-1)) );
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Blackman-harris window [harris:1978]
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
float blackmanharris(unsigned int _n,
                     unsigned int _N)
{
    // TODO test this function
    // TODO add reference
    float a0 = 0.35875f;
    float a1 = 0.48829f;
    float a2 = 0.14128f;
    float a3 = 0.01168f;
    float t = 2*M_PI*(float)_n / ((float)(_N-1));

    return a0 - a1*cosf(t) + a2*cosf(2*t) - a3*cosf(3*t);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 7th-order Blackman-harris window
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
float blackmanharris7(unsigned int _n, unsigned int _N)
{
	float a0 = 0.27105f;
	float a1 = 0.43329f;
	float a2 = 0.21812f;
	float a3 = 0.06592f;
	float a4 = 0.01081f;
	float a5 = 0.00077f;
	float a6 = 0.00001f;
	float t = 2*M_PI*(float)_n / ((float)(_N-1));

	return a0 - a1*cosf(  t) + a2*cosf(2*t) - a3*cosf(3*t)
			  + a4*cosf(4*t) - a5*cosf(5*t) + a6*cosf(6*t);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Flat-top window
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
float flattop(unsigned int _n, unsigned int _N)
{
	float a0 = 1.000f;
	float a1 = 1.930f;
	float a2 = 1.290f;
	float a3 = 0.388f;
	float a4 = 0.028f;
	float t = 2*M_PI*(float)_n / ((float)(_N-1));

	return a0 - a1*cosf(t) + a2*cosf(2*t) - a3*cosf(3*t) + a4*cosf(4*t);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Triangular window
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
float triangular(unsigned int _n,
                 unsigned int _N,
                 unsigned int _L)
{
	float _num   = (float)_n - (float)((_N-1)/2.0f);
	float _denom = ((float)_L)/2.0f;
	return 1.0 - fabsf(_num / _denom);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
// raised-cosine tapering window
//  _n      :   window index
//  _t      :   taper length
//  _N      :   full window length
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
float liquid_rcostaper_windowf(unsigned int _n,
                               unsigned int _t,
                               unsigned int _N)
{
    // reverse time for ramp-down section
    if (_n > _N - _t - 1)
        _n = _N - _n - 1;

    // return ramp or flat component
    return (_n < _t) ? 0.5f - 0.5f*cosf(M_PI*((float)_n + 0.5f) / (float)_t) : 1.0f;
}
