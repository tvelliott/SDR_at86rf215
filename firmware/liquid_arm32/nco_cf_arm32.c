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
// Numerically-controlled oscillator (nco) with internal phase-locked
// loop (pll) implementation
//

#include <math.h>
#include <complex.h>
#include <stdlib.h>
#include <stdio.h>

#define NCO_PLL_BANDWIDTH_DEFAULT   (0.1)
#define NCO_PLL_GAIN_DEFAULT        (1000)

#include "globals.h"
#include "nco_cf_arm32.h"


///////////////////////////////////////////////////////////////////////////////////////////////////////////
// create nco/vco object
///////////////////////////////////////////////////////////////////////////////////////////////////////////
struct nco_cf_s *  nco_create(int _type)
{
    struct nco_cf_s *q = (struct nco_cf_s * ) malloc(sizeof(struct nco_cf_s));
    q->type = _type;

    // initialize sine table
    unsigned int i;
    for (i=0; i<256; i++)
        q->sintab[i] = sinf(2.0f*M_PI*(float)(i)/256.0f);

    // set default pll bandwidth
    nco_pll_set_bandwidth(q, NCO_PLL_BANDWIDTH_DEFAULT);

    // set internal method
    if (q->type == LIQUID_NCO) {
        q->compute_sincos = &nco_compute_sincos_nco;
    } else if (q->type == LIQUID_VCO) {
        q->compute_sincos = &nco_compute_sincos_vco;
    } else {
			//default
        q->compute_sincos = &nco_compute_sincos_nco;
    }

    // reset object and return
    nco_reset(q);
    return q;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
// reset internal state of nco object
///////////////////////////////////////////////////////////////////////////////////////////////////////////
void nco_reset(struct nco_cf_s *  _q)
{
    _q->theta = 0;
    _q->d_theta = 0;

    // reset sine table index
    _q->index = 0;

    // set internal sine, cosine values
    _q->sine = 0;
    _q->cosine = 1;

    // reset pll filter state
    nco_pll_reset(_q);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
// set frequency of nco object
///////////////////////////////////////////////////////////////////////////////////////////////////////////
void nco_set_frequency(struct nco_cf_s *  _q,
                         float     _dtheta)
{
    _q->d_theta = _dtheta;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
// adjust frequency of nco object
///////////////////////////////////////////////////////////////////////////////////////////////////////////
void nco_adjust_frequency(struct nco_cf_s *  _q,
                            float _df)
{
    _q->d_theta += _df;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
// set phase of nco object, constraining phase
///////////////////////////////////////////////////////////////////////////////////////////////////////////
void nco_set_phase(struct nco_cf_s *  _q, float _phi)
{
    _q->theta = _phi;
    nco_constrain_phase(_q);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
// adjust phase of nco object, constraining phase
///////////////////////////////////////////////////////////////////////////////////////////////////////////
void nco_adjust_phase(struct nco_cf_s *  _q, float _dphi)
{
    _q->theta += _dphi;
    nco_constrain_phase(_q);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
// increment internal phase of nco object
///////////////////////////////////////////////////////////////////////////////////////////////////////////
void nco_step(struct nco_cf_s *  _q)
{
    _q->theta += _q->d_theta;
    nco_constrain_phase(_q);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
// get phase
///////////////////////////////////////////////////////////////////////////////////////////////////////////
float nco_get_phase(struct nco_cf_s *  _q)
{
    return _q->theta;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
// ge frequency
///////////////////////////////////////////////////////////////////////////////////////////////////////////
float nco_get_frequency(struct nco_cf_s *  _q)
{
    return _q->d_theta;
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////
// TODO : compute sine, cosine internally
///////////////////////////////////////////////////////////////////////////////////////////////////////////
float nco_sin(struct nco_cf_s *  _q)
{
    // compute internal sin, cos
    _q->compute_sincos(_q);

    // return resulting cosine component
    return _q->sine;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////
float nco_cos(struct nco_cf_s *  _q)
{
    // compute internal sin, cos
    _q->compute_sincos(_q);

    // return resulting cosine component
    return _q->cosine;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
// compute sin, cos of internal phase
///////////////////////////////////////////////////////////////////////////////////////////////////////////
void nco_sincos(struct nco_cf_s *  _q,
                  float *   _s,
                  float *   _c)
{
    // compute sine, cosine internally, calling implementation-
    // specific function (nco, vco)
    _q->compute_sincos(_q);

    // return result
    *_s = _q->sine;
    *_c = _q->cosine;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
// compute complex exponential of internal phase
///////////////////////////////////////////////////////////////////////////////////////////////////////////
void nco_cexpf(struct nco_cf_s *  _q,
                 float complex * _y)
{
    // compute sine, cosine internally, calling implementation-
    // specific function (nco, vco)
    _q->compute_sincos(_q);

    // set _y[0] to [cos(theta) + _Complex_I*sin(theta)]
    *_y = _q->cosine + _Complex_I*(_q->sine);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
// pll methods
// reset pll state, retaining base frequency
///////////////////////////////////////////////////////////////////////////////////////////////////////////
void nco_pll_reset(struct nco_cf_s *  _q)
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
// set pll bandwidth
///////////////////////////////////////////////////////////////////////////////////////////////////////////
void nco_pll_set_bandwidth(struct nco_cf_s *  _q,
                             float     _bw)
{
    _q->alpha = _bw;                // frequency proportion
    _q->beta  = sqrtf(_q->alpha);   // phase proportion
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
// advance pll phase
//  _q      :   nco object
//  _dphi   :   phase error
///////////////////////////////////////////////////////////////////////////////////////////////////////////
void nco_pll_step(struct nco_cf_s *  _q,
                    float     _dphi)
{
    // increase frequency proportional to error
    nco_adjust_frequency(_q, _dphi*_q->alpha);

    // increase phase proportional to error
    nco_adjust_phase(_q, _dphi*_q->beta);

    // constrain frequency
    //nco_constrain_frequency)(_q);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
// mixing functions
// Rotate input vector up by NCO angle, y = x exp{+j theta}
//  _q      :   nco object
//  _x      :   input sample
//  _y      :   output sample
///////////////////////////////////////////////////////////////////////////////////////////////////////////
void nco_mix_up(struct nco_cf_s *  _q,
                  float complex _x,
                  float complex *_y)
{
    // compute sine, cosine internally, calling implementation-
    // specific function (nco, vco)
    _q->compute_sincos(_q);

    // multiply _x by [cos(theta) + _Complex_I*sin(theta)]
    *_y = _x * (_q->cosine + _Complex_I*(_q->sine));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
// Rotate input vector down by NCO angle, y = x exp{-j theta}
//  _q      :   nco object
//  _x      :   input sample
//  _y      :   output sample
///////////////////////////////////////////////////////////////////////////////////////////////////////////
void nco_mix_down(struct nco_cf_s *  _q,
                    float complex _x,
                    float complex *_y)
{
    // compute sine, cosine internally
    _q->compute_sincos(_q);

    // multiply _x by [cos(-theta) + _Complex_I*sin(-theta)]
    *_y = _x * (_q->cosine - _Complex_I*(_q->sine));
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////
// Rotate input vector array up by NCO angle:
//      y(t) = x(t) exp{+j (f*t + theta)}
// TODO : implement NCO/VCO-specific versions
//  _q      :   nco object
//  _x      :   input array [size: _n x 1]
//  _y      :   output sample [size: _n x 1]
//  _n      :   number of input, output samples
///////////////////////////////////////////////////////////////////////////////////////////////////////////
void nco_mix_block_up(struct nco_cf_s *  _q,
                        float complex *_x,
                        float complex *_y,
                        unsigned int _n)
{
    unsigned int i;
    // FIXME: this method should be more efficient but is causing occasional
    //        errors so instead favor slower but more reliable algorithm
#if 0
    float theta =   _q->theta;
    float d_theta = _q->d_theta;
    for (i=0; i<_n; i++) {
        // multiply _x[i] by [cos(theta) + _Complex_I*sin(theta)]
        _y[i] = _x[i] * liquid_cexpjf(theta);
        
        theta += d_theta;
    }

    nco_set_phase)(_q, theta);
#else
    for (i=0; i<_n; i++) {
        // mix single sample up
        nco_mix_up(_q, _x[i], &_y[i]);

        // step NCO phase
        nco_step(_q);
    }
#endif
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
// Rotate input vector array down by NCO angle:
//      y(t) = x(t) exp{-j (f*t + theta)}
// TODO : implement NCO/VCO-specific versions
//  _q      :   nco object
//  _x      :   input array [size: _n x 1]
//  _y      :   output sample [size: _n x 1]
//  _n      :   number of input, output samples
///////////////////////////////////////////////////////////////////////////////////////////////////////////
void nco_mix_block_down(struct nco_cf_s *  _q,
                          float complex *_x,
                          float complex *_y,
                          unsigned int _n)
{
    unsigned int i;
    // FIXME: this method should be more efficient but is causing occasional
    //        errors so instead favor slower but more reliable algorithm
#if 0
    float theta =   _q->theta;
    float d_theta = _q->d_theta;
    for (i=0; i<_n; i++) {
        // multiply _x[i] by [cos(-theta) + _Complex_I*sin(-theta)]
        _y[i] = _x[i] * liquid_cexpjf(-theta);
        
        theta += d_theta;
    }

    nco_set_phase)(_q, theta);
#else
    for (i=0; i<_n; i++) {
        // mix single sample down
        nco_mix_down(_q, _x[i], &_y[i]);

        // step NCO phase
        nco_step(_q);
    }
#endif
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
// internal methods
//
// constrain frequency of NCO object to be in (-pi,pi)
///////////////////////////////////////////////////////////////////////////////////////////////////////////
void nco_constrain_frequency(struct nco_cf_s *  _q)
{
    if (_q->d_theta > M_PI)
        _q->d_theta -= 2*M_PI;
    else if (_q->d_theta < -M_PI)
        _q->d_theta += 2*M_PI;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
// constrain phase of NCO object to be in (-pi,pi)
///////////////////////////////////////////////////////////////////////////////////////////////////////////
void nco_constrain_phase(struct nco_cf_s *  _q)
{
    if (_q->theta > M_PI)
        _q->theta -= 2*M_PI;
    else if (_q->theta < -M_PI)
        _q->theta += 2*M_PI;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
// compute sin, cos of internal phase of nco
///////////////////////////////////////////////////////////////////////////////////////////////////////////
void nco_compute_sincos_nco(struct nco_cf_s *  _q)
{
    // assume phase is constrained to be in (-pi,pi)

    // compute index
    // NOTE : 40.743665 ~ 256 / (2*pi)
    // NOTE : add 512 to ensure positive value, add 0.5 for rounding precision
    // TODO : move away from floating-point specific code
    _q->index = ((unsigned int)((_q->theta)*40.743665f + 512.0f + 0.5f))&0xff;
    
    _q->sine = _q->sintab[_q->index];
    _q->cosine = _q->sintab[(_q->index+64)&0xff];
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////
// compute sin, cos of internal phase of vco
///////////////////////////////////////////////////////////////////////////////////////////////////////////
void nco_compute_sincos_vco(struct nco_cf_s *  _q)
{
    _q->sine   = sinf(_q->theta);
    _q->cosine = cosf(_q->theta);
}
