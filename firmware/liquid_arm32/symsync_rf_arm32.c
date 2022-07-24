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
// Symbol synchronizer
//
// References:
//  [Mengali:1997] Umberto Mengali and Aldo N. D'Andrea,
//      "Synchronization Techniques for Digital Receivers,"
//      Plenum Press, New York & London, 1997.
//  [harris:2001] frederic j. harris and Michael Rice,
//      "Multirate Digital Filters for Symbol Timing Synchronization
//      in Software Defined Radios," IEEE Journal on Selected Areas
//      of Communications, vol. 19, no. 12, December, 2001, pp.
//      2346-2357.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <complex.h>

#include "symsync_rf_arm32.h"
#include "firpfb_rf_arm32.h"
#include "iirfiltsos_rrrf_arm32.h"
#include "firdes_arm32.h"
#include "rrcos_arm32.h"
#include "window_rf_arm32.h"


////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// create synchronizer object from external coefficients
//  _k      : samples per symbol
//  _M      : number of filters in the bank
//  _h      : matched filter coefficients
//  _h_len  : length of matched filter
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
struct symsync_s *symsync_create( struct symsync_s *q, unsigned int _k, unsigned int _M, float *_h, unsigned int _h_len )
{
  float dh[SYMSYNC_HLEN + 1]; //temporary until firpfb is created   8 * 40 len for filter coef

	struct _symsync_internal *ssi = malloc( sizeof(struct _symsync_internal) );

  // set internal properties
  q->k    = _k;  // input samples per symbol
  q->npfb = _M;  // number of filters in the polyphase filter bank

  // set output rate (nominally 1, full decimation)
  symsync_set_output_rate( q, 1 );

  // set internal sub-filter length
  q->h_len = ( _h_len - 1 ) / q->npfb;

  //printf("\r\nsymsync h_len %d, _h_len %d", q->h_len, _h_len);

  // compute derivative filter
  float hdh_max = 0.0f;
  unsigned int i;
  for( i = 0; i < _h_len; i++ ) {
    if( i == 0 ) {
      dh[i] = _h[i + 1] - _h[_h_len - 1];
    } else if( i == _h_len - 1 ) {
      dh[i] = _h[0]   - _h[i - 1];
    } else {
      dh[i] = _h[i + 1] - _h[i - 1];
    }

    // find maximum of h*dh
    if( fabsf( _h[i]*dh[i] ) > hdh_max || i == 0 )
      hdh_max = fabsf( _h[i] * dh[i] );
  }

  //for( i = 0; i < _h_len; i++ ) {
  //printf("\r\nHf[%d]=%3.4f", i, Hf[i]);
  //}
  //for( i = 0; i < _h_len; i++ ) {
  //printf("\r\ndh[%d]=%3.4f", i, dh[i]);
  //}

  // apply scaling factor for normalized response
  for( i = 0; i < _h_len; i++ )
    dh[i] *= 0.06f / hdh_max;

  q->mf  = firpfb_rf_create( &(ssi->_mf_internal), q->npfb, _h, _h_len, &(ssi->_window_hf), ssi->_dprod_hf );
  q->dmf = firpfb_rf_create( &(ssi->_dmf_internal), q->npfb, dh, _h_len, &(ssi->_window_df), ssi->_dprod_df );

  // reset state and initialize loop filter
  q->A[0] = 1.0f;
  q->B[0] = 0.0f;
  q->A[1] = 0.0f;
  q->B[1] = 0.0f;
  q->A[2] = 0.0f;
  q->B[2] = 0.0f;
  q->pll = iirfiltsos_rrrf_create( &(ssi->_iirflt_internal), q->B, q->A );
  symsync_reset( q );
  symsync_set_lf_bw( q, 0.01f );

  // unlock loop control
  symsync_unlock( q );

  // return main object
  return q;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// create square-root Nyquist symbol synchronizer
//  _type   : filter type (e.g. LIQUID_RNYQUIST_RRC)
//  _k      : samples/symbol
//  _m      : symbol delay
//  _beta   : rolloff factor (0 < beta <= 1)
//  _M      : number of filters in the bank
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
struct symsync_s *symsync_create_rnyquist( struct symsync_s *q, int _type, unsigned int _k, unsigned int _m, float _beta, unsigned int _M )
{
  float Hf[SYMSYNC_HLEN + 1]; //temporary until firpfb is created   8 * 40 len for filter coef

  // allocate memory for filter coefficients
  unsigned int H_len = 2 * _M * _k * _m + 1;

  // design square-root Nyquist pulse-shaping filter
  liquid_firdes_prototype( _type, _k * _M, _m, _beta, 0, Hf );

  // create object and return
  return symsync_create( q, _k, _M, Hf, H_len );
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// create symsync using Kaiser filter interpolator; useful
// when the input signal has matched filter applied already
//  _k      : input samples/symbol
//  _m      : symbol delay
//  _beta   : rolloff factor, beta in (0,1]
//  _M      : number of filters in the bank
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
struct symsync_s *symsync_create_kaiser( struct symsync_s *q, unsigned int _k, unsigned int _m, float _beta, unsigned int _M )
{
  float Hf[SYMSYNC_HLEN + 1]; //temporary until firpfb is created   8 * 40 len for filter coef

  // allocate memory for filter coefficients
  unsigned int H_len = 2 * _M * _k * _m + 1;

  // design interpolating filter whose bandwidth is outside the cut-off
  // frequency of input signal
  // TODO: use _beta to compute more accurate cut-off frequency
  float fc = 0.75f;   // filter cut-off frequency (nominal)
  float As = 40.0f;   // filter stop-band attenuation
  liquid_firdes_kaiser( H_len, fc / ( float )( _k * _M ), As, 0.0f, Hf );

  // create object and return
  return symsync_create( q, _k, _M, Hf, H_len );
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// reset symsync internal state
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void symsync_reset( struct symsync_s *_q )
{
  // reset polyphase filterbank
  firpfb_rf_reset( _q->mf );

  // reset counters, etc.
  _q->rate          = ( float )_q->k / ( float )_q->k_out;
  _q->del           = _q->rate;
  _q->b             =   0;    // filterbank index
  _q->bf            = 0.0f;   // filterbank index (soft value)
  _q->tau           = 0.0f;   // instantaneous timing estimate
  _q->tau_decim     = 0.0f;   // instantaneous timing estaimte (decimated)
  _q->q             = 0.0f;   // timing error
  _q->q_hat         = 0.0f;   // filtered timing error
  _q->decim_counter = 0;      // decimated output counter

  // reset timing phase-locked loop filter
  iirfiltsos_rrrf_reset( _q->pll );
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// lock synchronizer object
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void symsync_lock( struct symsync_s *_q )
{
  _q->is_locked = 1;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// unlock synchronizer object
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void symsync_unlock( struct symsync_s *_q )
{
  _q->is_locked = 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// set synchronizer output rate (samples/symbol)
//  _q      :   synchronizer object
//  _k_out  :   output samples/symbol
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void symsync_set_output_rate( struct symsync_s *_q, unsigned int _k_out )
{
  // set output rate
  _q->k_out = _k_out;

  _q->rate = ( float )_q->k / ( float )_q->k_out;
  _q->del  = _q->rate;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
// set synchronizer loop filter bandwidth
//  _q      :   synchronizer object
//  _bt     :   loop bandwidth
////////////////////////////////////////////////////////////////////////////////////////////////////////
void symsync_set_lf_bw( struct symsync_s *_q, float _bt )
{
  // compute filter coefficients from bandwidth
  float alpha = 1.000f - _bt;
  float beta  = 0.220f * _bt;
  float a     = 0.500f;
  float b     = 0.495f;

  _q->B[0] = beta;
  _q->B[1] = 0.00f;
  _q->B[2] = 0.00f;

  _q->A[0] = 1.00f - a * alpha;
  _q->A[1] = -b * alpha;
  _q->A[2] = 0.00f;

  // set internal parameters of 2nd-order IIR filter
  iirfiltsos_rrrf_set_coefficients( _q->pll, _q->B, _q->A );

  // update rate adjustment factor
  _q->rate_adjustment = 0.5 * _bt;
}

/////////////////////////////////////////////////////////////
// return instantaneous fractional timing offset estimate
/////////////////////////////////////////////////////////////
float symsync_get_tau( struct symsync_s *_q )
{
  return _q->tau_decim;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// execute synchronizer on input data array
//  _q      : synchronizer object
//  _x      : input data array
//  _nx     : number of input samples
//  _y      : output data array
//  _ny     : number of samples written to output buffer
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void symsync_execute( struct symsync_s *_q, float *_x, unsigned int _nx, float *_y, unsigned int *_ny )
{
  unsigned int i, ny = 0, k = 0;
  for( i = 0; i < _nx; i++ ) {
    symsync_step( _q, _x[i], &_y[ny], &k );
    ny += k;
    //printf("%u\n",k);
  }
  *_ny = ny;
}

//
// internal methods
//

///////////////////////////////////////////////////////////////////////////////////////////////////////
// step synchronizer with single input sample
//  _q      : symsync object
//  _x      : input sample
//  _y      : output sample array pointer
//  _ny     : number of output samples written
///////////////////////////////////////////////////////////////////////////////////////////////////////
void symsync_step( struct symsync_s *_q, float _x, float *_y, unsigned int *_ny )
{
  // push sample into MF and dMF filterbanks
  firpfb_rf_push( _q->mf,  _x );
  firpfb_rf_push( _q->dmf, _x );

  // matched and derivative matched-filter outputs
  float mf; // matched filter output
  float dmf; // derivative matched filter output

  unsigned int n = 0;

  // continue loop until filterbank index rolls over
  while( _q->b < _q->npfb ) {

    // compute filterbank output
    firpfb_rf_execute( _q->mf, _q->b, &mf );

    // scale output by samples/symbol
    _y[n] = mf / ( float )( _q->k );

    // check output count and determine if this is 'ideal' timing output
    if( _q->decim_counter == _q->k_out ) {
      // reset counter
      _q->decim_counter = 0;

      // if synchronizer is locked, don't update internal timing offset
      if( _q->is_locked )
        continue;

      // compute dMF output
      firpfb_rf_execute( _q->dmf, _q->b, &dmf );

      // update internal state
      symsync_advance_internal_loop( _q, mf, dmf );
      _q->tau_decim = _q->tau;    // save return value
    }

    // increment decimation counter
    _q->decim_counter++;

    // update states
    _q->tau += _q->del;                     // instantaneous fractional offset
    _q->bf  = _q->tau * ( float )( _q->npfb ); // filterbank index (soft)
    _q->b   = ( int )roundf( _q->bf );      // filterbank index
    n++;                                    // number of output samples

    //printf("\r\nq,%3.4f,q_hat,%3.4f", _q->q, _q->q_hat);
  }

  // filterbank index rolled over; update states
  _q->tau -= 1.0f;                // instantaneous fractional offset
  _q->bf  -= ( float )( _q->npfb ); // filterbank index (soft)
  _q->b   -= _q->npfb;            // filterbank index

  // set output number of samples written
  *_ny = n;
}

///////////////////////////////////////////////////////////////////////////////////////////
// advance synchronizer's internal loop filter
//  _q      : synchronizer object
//  _mf     : matched-filter output
//  _dmf    : derivative matched-filter output
///////////////////////////////////////////////////////////////////////////////////////////
void symsync_advance_internal_loop( struct symsync_s *_q, float _mf, float _dmf )
{
  // TODO : use more efficient method to compute this
  _q->q = crealf( conjf( _mf * _Complex_I ) * _dmf * _Complex_I ); // [Mengali:1997] Eq.~(8.3.5)

  // constrain timing error
  if( _q->q >  1.0f ) _q->q =  1.0f;       // clip large positive values
  else if( _q->q < -1.0f ) _q->q = -1.0f;  // clip large negative values

  //  2. filter error signal through timing loop filter: retain large
  //     portion of old estimate and small percent of new estimate
  iirfiltsos_rrrf_execute( _q->pll, _q->q, &_q->q_hat );

  // 3. update rate and timing phase
  _q->rate += _q->rate_adjustment * _q->q_hat;
  _q->del   = _q->rate + _q->q_hat;

}
