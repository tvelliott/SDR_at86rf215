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
// Frequency modulator
//

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include <complex.h>

#include "globals.h"
#include "freqmod_arm32.h"


/////////////////////////////////////////////////////////////////////////////////////////////////////////
// create freqmod object
//  _kf     :   modulation factor
/////////////////////////////////////////////////////////////////////////////////////////////////////////
struct freqmod_s *  freqmod_create(float _kf)
{
    // create main object memory
    struct freqmod_s *q = malloc(sizeof(struct freqmod_s));

    // set modulation factor
    q->kf  = _kf;
    q->ref = q->kf * (1<<16);

    // initialize look-up table
    q->sincos_table_len = 1024;
    q->sincos_table     = (float complex *) malloc( q->sincos_table_len*sizeof(float complex) );
    unsigned int i;
    for (i=0; i<q->sincos_table_len; i++) {
        q->sincos_table[i] = cexpf(_Complex_I*2*M_PI*(float)i / (float)(q->sincos_table_len) );
    }

    // reset modem object
    freqmod_reset(q);

    // return object
    return q;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// reset modem object
/////////////////////////////////////////////////////////////////////////////////////////////////////////
void freqmod_reset(struct freqmod_s *_q)
{
    // reset phase accumulation
    _q->sincos_table_phase = 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// modulate sample
//  _q      :   frequency modulator object
//  _m      :   message signal m(t)
//  _s      :   complex baseband signal s(t)
/////////////////////////////////////////////////////////////////////////////////////////////////////////
void freqmod_modulate(struct freqmod_s *_q,
                        float           _m,
                        float complex *_s)
{
    // accumulate phase; this wraps around a 16-bit boundary and ensures
    // that negative numbers are mapped to positive numbers
    _q->sincos_table_phase =
        (_q->sincos_table_phase + (1<<16) + (int)roundf(_q->ref*_m)) & 0xffff;

    // compute table index: mask out 10 most significant bits with rounding
    // (adding 0x0020 effectively rounds to nearest value with 10 bits of
    // precision)
    unsigned int index = ( (_q->sincos_table_phase+0x0020) >> 6) & 0x03ff;

    // return table value at index
    *_s = _q->sincos_table[index];
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// modulate block of samples
//  _q      :   frequency modulator object
//  _m      :   message signal m(t), [size: _n x 1]
//  _n      :   number of input, output samples
//  _s      :   complex baseband signal s(t) [size: _n x 1]
/////////////////////////////////////////////////////////////////////////////////////////////////////////
void freqmod_modulate_block(struct freqmod_s *_q,
                              float *_m,
                              unsigned int _n,
                              float complex *_s)
{
    // TODO: implement more efficient method
    unsigned int i;
    for (i=0; i<_n; i++)
        freqmod_modulate(_q, _m[i], &_s[i]);
}
