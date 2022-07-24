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
// Arbitrary resampler
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <complex.h>

#include "firdes_arm32.h"
#include "firpfb_rf_arm32.h"
#include "resamp_rf_arm32.h"
#include "resamp_config.h"


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// create arbitrary resampler
//  _rate   :   resampling rate
//  _m      :   prototype filter semi-length
//  _fc     :   prototype filter cutoff frequency, fc in (0, 0.5)
//  _As     :   prototype filter stop-band attenuation [dB] (e.g. 60)
//  _npfb   :   number of filters in polyphase filterbank
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
struct resamp_rf_s *  resamp_rf_create( struct resamp_rf_s *q,
												 float        _rate,
                         unsigned int _m,
                         float        _fc,
                         float        _As,
                         unsigned int _npfb)
{
		struct _resamp_rf_internal *rsi = malloc( sizeof(struct _resamp_rf_internal) );

    // set rate using formal method (specifies output stride
    // value 'del')
    resamp_rf_set_rate(q, _rate);

    // set properties
    //q->m    = _m;       // prototype filter semi-length
		q->m    = RESAMP_RF_M; 

    q->fc   = _fc;      // prototype filter cutoff frequency
    q->As   = _As;      // prototype filter stop-band attenuation

    //q->npfb = _npfb;    // number of filters in bank
		q->npfb = RESAMP_RF_NFILTERS; 

    // design filter	 (define in resamp_config.h)
    //unsigned int n = 2*q->m*q->npfb+1;

    float hf[RESAMP_RF_HLEN];
    float h[RESAMP_RF_HLEN];

    liquid_firdes_kaiser(RESAMP_RF_HLEN, q->fc/((float)(q->npfb)), q->As,0.0f, hf);

    // normalize filter coefficients by DC gain
    unsigned int i;
    float gain=0.0f;
    for (i=0; i<RESAMP_RF_HLEN; i++)
        gain += hf[i];
    gain = (q->npfb)/(gain);

    // copy to type-specific array, applying gain
    for (i=0; i<RESAMP_RF_HLEN; i++) {
        h[i] = hf[i]*gain;
		}
    q->f = firpfb_rf_create(&(rsi->_firpfb_rf_internal), q->npfb, h, RESAMP_RF_HLEN-1, &(rsi->_window), rsi->_dprod);

    // reset object and return
    resamp_rf_reset(q);
    return q;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// reset resampler object
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void resamp_rf_reset(struct resamp_rf_s *_q)
{
    // clear filterbank
    firpfb_rf_reset(_q->f);

    // reset states
    _q->state = RESAMP_RF_STATE_INTERP;// input/output sample state
    _q->tau   = 0.0f;           // accumulated timing phase
    _q->bf    = 0.0f;           // soft-valued filterbank index
    _q->b     = 0;              // base filterbank index
    _q->mu    = 0.0f;           // fractional filterbank interpolation value

    _q->y0    = 0;              // filterbank output at index b
    _q->y1    = 0;              // filterbank output at index b+1
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// get resampler filter delay (semi-length m)
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
unsigned int resamp_rf_get_delay(struct resamp_rf_s *_q)
{
    return _q->m;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// set rate of arbitrary resampler
//  _q      : resampling object
//  _rate   : new sampling rate, _rate > 0
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void resamp_rf_set_rate(struct resamp_rf_s *_q,
                       float    _rate)
{
    // set internal rate
    _q->rate = _rate;

    // set output stride
    _q->del = 1.0f / _q->rate;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// get rate of arbitrary resampler
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
float resamp_rf_get_rate(struct resamp_rf_s *_q)
{
    return _q->rate;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// adjust resampling rate
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void resamp_rf_adjust_rate(struct resamp_rf_s *_q,
                          float    _gamma)
{
    // adjust internal rate
    _q->rate *= _gamma;

    // set output stride
    _q->del = 1.0f / _q->rate;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// set resampling timing phase
//  _q      : resampling object
//  _tau    : sample timing
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void resamp_rf_set_timing_phase(struct resamp_rf_s *_q,
                               float    _tau)
{
    // set internal timing phase
    _q->tau = _tau;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// adjust resampling timing phase
//  _q      : resampling object
//  _delta  : sample timing adjustment
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void resamp_rf_adjust_timing_phase(struct resamp_rf_s *_q,
                                  float    _delta)
{
    // adjust internal timing phase
    _q->tau += _delta;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// run arbitrary resampler
//  _q          :   resampling object
//  _x          :   single input sample
//  _y          :   output array
//  _num_written:   number of samples written to output
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void resamp_rf_execute(struct resamp_rf_s *_q,
                      float _x,
                      float *_y,
                      unsigned int *_num_written)
{
    // push input sample into filterbank
    firpfb_rf_push(_q->f, _x);
    unsigned int n=0;
    
    while (_q->b < _q->npfb) {

        switch (_q->state) {
        case RESAMP_RF_STATE_BOUNDARY:
            // compute filterbank output
            firpfb_rf_execute(_q->f, 0, &_q->y1);

            // interpolate
            _y[n++] = (1.0f - _q->mu)*_q->y0 + _q->mu*_q->y1;
        
            // update timing state
            resamp_rf_update_timing_state(_q);

            _q->state = RESAMP_RF_STATE_INTERP;
            break;

        case RESAMP_RF_STATE_INTERP:
            // compute output at base index
            firpfb_rf_execute(_q->f, _q->b, &_q->y0);

            // check to see if base index is last filter in the bank, in
            // which case the resampler needs an additional input sample
            // to finish the linear interpolation process
            if (_q->b == _q->npfb-1) {
                // last filter: need additional input sample
                _q->state = RESAMP_RF_STATE_BOUNDARY;
            
                // set index to indicate new sample is needed
                _q->b = _q->npfb;
            } else {
                // do not need additional input sample; compute
                // output at incremented base index
                firpfb_rf_execute(_q->f, _q->b+1, &_q->y1);

                // perform linear interpolation between filterbank outputs
                _y[n++] = (1.0f - _q->mu)*_q->y0 + _q->mu*_q->y1;

                // update timing state
                resamp_rf_update_timing_state(_q);
            }
            break;
        default:
            _q->state = RESAMP_RF_STATE_INTERP;	//shouldn't happen, but probably will somehow
						break;
        }
    }

    // decrement timing phase by one sample
    _q->tau -= 1.0f;
    _q->bf  -= (float)(_q->npfb);
    _q->b   -= _q->npfb;

    // specify number of samples written
    *_num_written = n;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// execute arbitrary resampler on a block of samples
//  _q              :   resamp object
//  _x              :   input buffer [size: _nx x 1]
//  _nx             :   input buffer
//  _y              :   output sample array (pointer)
//  _ny             :   number of samples written to _y
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void resamp_rf_execute_block(struct resamp_rf_s *_q,
                            float *_x,
                            unsigned int   _nx,
                            float *_y,
                            unsigned int * _ny)
{
    // initialize number of output samples to zero
    unsigned int ny = 0;

    // number of samples written for each individual iteration
    unsigned int num_written;

    // iterate over each input sample
    unsigned int i;
    for (i=0; i<_nx; i++) {
        // run resampler on single input
        resamp_rf_execute(_q, _x[i], &_y[ny], &num_written);

        // update output counter
        ny += num_written;
    }

    // set return value for number of output samples written
    *_ny = ny;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// internal methods
// 
// update timing state; increment output timing stride and
// quantize filterbank indices
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void resamp_rf_update_timing_state(struct resamp_rf_s *  _q)
{
    // update high-resolution timing phase
    _q->tau += _q->del;

    // convert to high-resolution filterbank index 
    _q->bf  = _q->tau * (float)(_q->npfb);

    // split into integer filterbank index and fractional interpolation
    _q->b   = (int)floorf(_q->bf);      // base index
    _q->mu  = _q->bf - (float)(_q->b);  // fractional index
}
