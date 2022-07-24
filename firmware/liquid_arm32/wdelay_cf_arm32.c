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
// windowed delay, defined by macro
//

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <complex.h>

#include "wdelay_cf_arm32.h"


//////////////////////////////////////////////////////////////////////////////////////////////////
// create delay buffer object with '_delay' samples
//////////////////////////////////////////////////////////////////////////////////////////////////
struct wdelay_cf_s *  wdelay_cf_create(unsigned int _delay)
{
    // create main object
    struct wdelay_cf_s *  q = (struct wdelay_cf_s * ) malloc(sizeof(struct wdelay_cf_s));

    // set internal values
    q->delay = _delay;

    // allocte memory
    q->v = (float complex*) malloc((q->delay)*sizeof(float complex));
    q->read_index = 0;

    // clear window
    wdelay_cf_reset(q);

    return q;
}

//////////////////////////////////////////////////////////////////////////////////////////////////
// clear/reset state of object
//////////////////////////////////////////////////////////////////////////////////////////////////
void wdelay_cf_reset(struct wdelay_cf_s *  _q)
{
    _q->read_index = 0;
    memset(_q->v, 0, (_q->delay)*sizeof(float complex));
}

//////////////////////////////////////////////////////////////////////////////////////////////////
// read delayed sample from delay buffer object
//  _q  :   delay buffer object
//  _v  :   value of delayed element
//////////////////////////////////////////////////////////////////////////////////////////////////
void wdelay_cf_read(struct wdelay_cf_s *  _q,
                   float complex *      _v)
{
    // return value at end of buffer
    *_v = _q->v[_q->read_index];
}

//////////////////////////////////////////////////////////////////////////////////////////////////
// push new sample into delay buffer object
//  _q  :   delay buffer object
//  _v  :   new value to be added to buffer
//////////////////////////////////////////////////////////////////////////////////////////////////
void wdelay_cf_push(struct wdelay_cf_s *  _q,
                   float complex        _v)
{
    // append value to end of buffer
    _q->v[_q->read_index] = _v;

    // increment index
    _q->read_index++;

    // wrap around pointer
    _q->read_index %= _q->delay;
}
