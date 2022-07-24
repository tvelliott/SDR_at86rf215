
//
//t.elliott converted from original liquid-dsp code to something that would be usable on the STM32H7 series
//
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <complex.h>

#include "dotprod_rf_arm32.h"
#include "firpfb_rf_arm32.h"
#include "firdes_arm32.h"
#include "window_rf_arm32.h"

///////////////////////////////////////////////////////////////////////////////
// create firpfb from external coefficients
//  _M      : number of filters in the bank
//  _h      : coefficients [size: _M*_h_len x 1]
//  _h_len  : filter delay (symbols)
///////////////////////////////////////////////////////////////////////////////
struct firpfb_rf_s *firpfb_rf_create( struct firpfb_rf_s *q, unsigned int _M, float *_h, unsigned int _h_len, struct window_rf_s *_window, struct dotprod_rf_s *_dotprod )
{

  //8 filters, sub-len 40
  float h_sub[SYMSYNC_H_SUBLEN];  //temporary.  gets copied to dotproduct object

  // set user-defined parameters
  q->num_filters = _M;
  q->h_len       = _h_len;

  // generate bank of sub-samped filters
  // length of each sub-sampled filter
  unsigned int h_sub_len = _h_len / q->num_filters;

  unsigned int i, n;
  for( i = 0; i < q->num_filters; i++ ) {
    for( n = 0; n < h_sub_len; n++ ) {
      // load filter in reverse order
      h_sub[( h_sub_len - n - 1 )] = _h[i + n * ( q->num_filters )];

    }
    // create dot product object
    q->dp[i] = dotprod_rf_create( _dotprod++, h_sub, h_sub_len );

  }

  // save sub-sampled filter length
  q->h_sub_len = h_sub_len;

  // create window buffer
  q->w = window_rf_create( _window, q->h_sub_len );

  // set default scaling
  q->scale = 1;

  // reset object and return
  firpfb_rf_reset( q );
  return q;
}

/*
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// create firpfb using kaiser window
//  _M      : number of filters in the bank
//  _m      : filter semi-length [samples]
//  _fc     : filter cut-off frequency 0 < _fc < 0.5
//  _As     : filter stop-band suppression [dB]
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
struct firpfb_rf_s * firpfb_create_kaiser( struct firpfb_rf_s *q, unsigned int _M, unsigned int _m, float _fc, float _As )
{

  // design filter using kaiser window
  unsigned int H_len = 2 * _M * _m + 1;
  liquid_firdes_kaiser( H_len, _fc / ( float )_M, _As, 0.0f, Hf );

  // return filterbank object
  return firpfb_create( q, _M, Hf, H_len );
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// create square-root Nyquist filterbank
//  _type   :   filter type (e.g. LIQUID_RNYQUIST_RRC)
//  _M      :   number of filters in the bank
//  _k      :   samples/symbol _k > 1
//  _m      :   filter delay (symbols), _m > 0
//  _beta   :   excess bandwidth factor, 0 < _beta < 1
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
struct firpfb_rf_s * firpfb_rf_create_rnyquist( struct firpfb_rf_s *q, int _type, unsigned int _M, unsigned int _k, unsigned int _m, float _beta )
{

  // generate square-root Nyquist filter
  unsigned int H_len = 2 * _M * _k * _m + 1;
  liquid_firdes_prototype( _type, _M * _k, _m, _beta, 0, Hf );

  // return filterbank object
  return firpfb_rf_create( q, _M, Hf, H_len );
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// create firpfb derivative square-root Nyquist filterbank
//  _type   :   filter type (e.g. LIQUID_RNYQUIST_RRC)
//  _M      :   number of filters in the bank
//  _k      :   samples/symbol _k > 1
//  _m      :   filter delay (symbols), _m > 0
//  _beta   :   excess bandwidth factor, 0 < _beta < 1
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
struct firpfb_rf_s * firpfb_rf_create_drnyquist( struct firpfb_rf_s *q, int _type, unsigned int _M, unsigned int _k, unsigned int _m, float _beta )
{

  // generate square-root Nyquist filter
  unsigned int H_len = 2 * _M * _k * _m + 1;
  liquid_firdes_prototype( _type, _M * _k, _m, _beta, 0, Hf );

  // compute derivative filter
  float HdH_max = 0.0f;
  unsigned int i;
  for( i = 0; i < H_len; i++ ) {
    if( i == 0 ) {
      dHf[i] = Hf[i + 1] - Hf[H_len - 1];
    } else if( i == H_len - 1 ) {
      dHf[i] = Hf[0]   - Hf[i - 1];
    } else {
      dHf[i] = Hf[i + 1] - Hf[i - 1];
    }

    // find maximum of h*dh
    if( fabsf( Hf[i]*dHf[i] ) > HdH_max )
      HdH_max = fabsf( Hf[i] * dHf[i] );
  }

  // and apply scaling factor for normalized response
  for( i = 0; i < H_len; i++ )
    dHf[i] = dHf[i] * 0.06f / HdH_max;

  // return filterbank object
  return firpfb_rf_create( q, _M, dHf, H_len );
}
*/


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// clear/reset firpfb object internal state
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void firpfb_rf_reset( struct firpfb_rf_s *_q )
{
  window_rf_reset( _q->w );
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// set output scaling for filter
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void firpfb_rf_set_scale( struct firpfb_rf_s *_q, float _scale )
{
  _q->scale = _scale;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// push sample into firpfb internal buffer
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void firpfb_rf_push( struct firpfb_rf_s *_q, float _x )
{
  // push value into window buffer
  window_rf_push( _q->w, _x );
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// execute the filter on internal buffer and coefficients
//  _q      : firpfb object
//  _i      : index of filter to use
//  _y      : pointer to output sample
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void firpfb_rf_execute( struct firpfb_rf_s *_q, unsigned int _i, float *_y )
{
  // read buffer
  float *r;
  window_rf_read( _q->w, &r );

  // execute dot product
  dotprod_rf_execute( _q->dp[_i], r, _y );

  // apply scaling factor
  *_y *= _q->scale;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// execute the filter on a block of input samples; the
// input and output buffers may be the same
//  _q      : firpfb object
//  _i      : index of filter to use
//  _x      : pointer to input array [size: _n x 1]
//  _n      : number of input, output samples
//  _y      : pointer to output array [size: _n x 1]
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void firpfb_rf_execute_block( struct firpfb_rf_s *_q, unsigned int _i, float *_x, unsigned int _n, float *_y )
{
  unsigned int i;
  for( i = 0; i < _n; i++ ) {
    // push sample into filter
    firpfb_rf_push( _q, _x[i] );

    // compute output at appropriate index
    firpfb_rf_execute( _q, _i, &_y[i] );
  }
}
