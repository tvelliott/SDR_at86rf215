
//
//t.elliott converted from original liquid-dsp code to something that would be usable on the STM32H7 series
//
/*
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
// Amplitude modulator/demodulator
//

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <complex.h>

#include "globals.h"
#include "firhilb_arm32.h"
#include "ampmodem_arm32.h"
#include "nco_cf_arm32.h"


////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// create ampmodem object
//  _m                  :   modulation index
//  _fc                 :   carrier frequency
//  _type               :   AM type (e.g. LIQUID_AMPMODEM_DSB)
//  _suppressed_carrier :   carrier suppression flag
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
struct ampmodem_s * ampmodem_create(float _m,
                         float _fc,
                         int _type,
                         int _suppressed_carrier)
{
    struct ampmodem_s *q = malloc(sizeof(struct ampmodem_s));
    q->type = _type;
    q->m    = _m;
    q->fc   = _fc;
    q->suppressed_carrier = (_suppressed_carrier == 0) ? 0 : 1;

    // create nco, pll objects
    q->oscillator = nco_create(LIQUID_NCO);
    nco_set_frequency(q->oscillator, 2*M_PI*q->fc);
    
    nco_pll_set_bandwidth(q->oscillator,0.05f);

    // suppressed carrier
    q->ssb_alpha = 0.01f;
    q->ssb_q_hat = 0.0f;

    // single side-band
    q->hilbert = firhilb_create(9, 60.0f);

    // double side-band

    ampmodem_reset(q);

    return q;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ampmodem_reset(struct ampmodem_s * _q)
{
    // single side-band
    _q->ssb_q_hat = 0.5f;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ampmodem_modulate(struct ampmodem_s *_q,
                       float _x,
                       float complex *_y)
{
    float complex x_hat = 0.0f;
    float complex y_hat;

    if (_q->type == LIQUID_AMPMODEM_DSB) {
        x_hat = _x;
    } else {
        // push through Hilbert transform
        // LIQUID_AMPMODEM_USB:
        // LIQUID_AMPMODEM_LSB: conjugate Hilbert transform output
        firhilb_r2c_execute(_q->hilbert, _x, &x_hat);

        if (_q->type == LIQUID_AMPMODEM_LSB)
            x_hat = conjf(x_hat);
    }

    if (_q->suppressed_carrier)
        y_hat = x_hat;
    else
        y_hat = 0.5f*(x_hat + 1.0f);
    
    // mix up
    nco_mix_up(_q->oscillator, y_hat, _y);
    nco_step(_q->oscillator);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// modulate block of samples
//  _q      :   ampmodem object
//  _m      :   message signal m(t), [size: _n x 1]
//  _n      :   number of input, output samples
//  _s      :   complex baseband signal s(t) [size: _n x 1]
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ampmodem_modulate_block(struct ampmodem_s *_q,
                             float *_m,
                             unsigned int _n,
                             float complex *_s)
{
    // TODO: implement more efficient method
    unsigned int i;
    for (i=0; i<_n; i++)
        ampmodem_modulate(_q, _m[i], &_s[i]);
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ampmodem_demodulate(struct ampmodem_s *_q,
                         float complex _y,
                         float *_x)
{

    if (_q->suppressed_carrier) {
        // single side-band suppressed carrier
        if (_q->type != LIQUID_AMPMODEM_DSB) {
            *_x = crealf(_y);
            return;
        }

        // coherent demodulation
        
        // mix signal down
        float complex y_hat;
        nco_mix_down(_q->oscillator, _y, &y_hat);

        // compute phase error
        float phase_error = tanhf( crealf(y_hat) * cimagf(y_hat) );

        // adjust nco, pll objects
        nco_pll_step(_q->oscillator, phase_error);

        // step NCO
        nco_step(_q->oscillator);

        // set output
        *_x = crealf(y_hat);
    } else {
        // non-coherent demodulation (peak detector)
        float t = cabsf(_y);

        // remove DC bias
        _q->ssb_q_hat = (    _q->ssb_alpha)*t +
                        (1 - _q->ssb_alpha)*_q->ssb_q_hat;
        *_x = 2.0f*(t - _q->ssb_q_hat);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// demodulate block of samples
//  _q      :   frequency demodulator object
//  _r      :   received signal r(t) [size: _n x 1]
//  _n      :   number of input, output samples
//  _m      :   message signal m(t), [size: _n x 1]
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ampmodem_demodulate_block(struct ampmodem_s *_q,
                               float complex *_r,
                               unsigned int _n,
                               float *_m)
{
    // TODO: implement more efficient method
    unsigned int i;
    for (i=0; i<_n; i++)
        ampmodem_demodulate(_q, _r[i], &_m[i]);
}
