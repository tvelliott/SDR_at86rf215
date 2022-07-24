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
// Design root-Nyquist Kaiser filter
//
// References
//  [Vaidyanathan:1993] Vaidyanathan, P. P., "Multirate Systems and
//      Filter Banks," 1993, Prentice Hall, Section 3.2.1
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <complex.h>

#include "firdes_arm32.h"
#include "rkaiser_arm32.h"

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Design frequency-shifted root-Nyquist filter based on
// the Kaiser-windowed sinc.
//
//  _k      :   filter over-sampling rate (samples/symbol)
//  _m      :   filter delay (symbols)
//  _beta   :   filter excess bandwidth factor (0,1)
//  _dt     :   filter fractional sample delay
//  _h      :   resulting filter [size: 2*_k*_m+1]
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void liquid_firdes_rkaiser( unsigned int _k,
                            unsigned int _m,
                            float _beta,
                            float _dt,
                            float *_h )
{
  // simply call internal method and ignore output rho value
  float rho;
  //liquid_firdes_rkaiser_bisection(_k,_m,_beta,_dt,_h,&rho);
  liquid_firdes_rkaiser_quadratic( _k, _m, _beta, _dt, _h, &rho );
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Design frequency-shifted root-Nyquist filter based on
// the Kaiser-windowed sinc using approximation for rho.
//
//  _k      :   filter over-sampling rate (samples/symbol)
//  _m      :   filter delay (symbols)
//  _beta   :   filter excess bandwidth factor (0,1)
//  _dt     :   filter fractional sample delay
//  _h      :   resulting filter [size: 2*_k*_m+1]
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void liquid_firdes_arkaiser( unsigned int _k,
                             unsigned int _m,
                             float _beta,
                             float _dt,
                             float *_h )
{

#if 0
  // compute bandwidth adjustment estimate
  float rho_hat = rkaiser_approximate_rho( _m, _beta ); // bandwidth correction factor
#else
  // rho ~ c0 + c1*log(_beta) + c2*log^2(_beta)

  // c0 ~ 0.762886 + 0.067663*log(m)
  // c1 ~ 0.065515
  // c2 ~ log( 1 - 0.088*m^-1.6 )

  float c0 = 0.762886 + 0.067663 * logf( _m );
  float c1 = 0.065515;
  float c2 = logf( 1 - 0.088 * powf( _m, -1.6 ) );

  float log_beta = logf( _beta );

  float rho_hat = c0 + c1 * log_beta + c2 * log_beta * log_beta;

  // ensure range is valid
  if( rho_hat <= 0.0f || rho_hat >= 1.0f )
    rho_hat = rkaiser_approximate_rho( _m, _beta );
#endif

  unsigned int n = 2 * _k * _m + 1;               // filter length
  float kf = ( float )_k;                         // samples/symbol (float)
  float del = _beta * rho_hat / kf;               // transition bandwidth
  float As = estimate_req_filter_As( del, n );    // stop-band suppression
  float fc  = 0.5f * ( 1 + _beta * ( 1.0f - rho_hat ) ) / kf; // filter cutoff

#if DEBUG_RKAISER
  printf( "rho-hat : %12.8f (compare to %12.8f)\n", rho_hat, rkaiser_approximate_rho( _m, _beta ) );
  printf( "fc      : %12.8f\n", fc );
  printf( "delta-f : %12.8f\n", del );
  printf( "As      : %12.8f dB\n", As );
  printf( "alpha   : %12.8f\n", kaiser_beta_As( As ) );
#endif

  // compute filter coefficients
  liquid_firdes_kaiser( n, fc, As, _dt, _h );

  // normalize coefficients
  float e2 = 0.0f;
  unsigned int i;
  for( i = 0; i < n; i++ ) e2 += _h[i] * _h[i];
  for( i = 0; i < n; i++ ) _h[i] *= sqrtf( _k / e2 );
}

// Find approximate bandwidth adjustment factor rho based on
// filter delay and desired excess bandwdith factor.
//
//  _m      :   filter delay (symbols)
//  _beta   :   filter excess bandwidth factor (0,1)
float rkaiser_approximate_rho( unsigned int _m,
                               float _beta )
{
  // compute bandwidth adjustment estimate
  float c0 = 0.0f, c1 = 0.0f, c2 = 0.0f;
  switch( _m ) {
  case 1:
    c0 = 0.75749731;
    c1 = 0.06134303;
    c2 = -0.08729663;
    break;
  case 2:
    c0 = 0.81151861;
    c1 = 0.07437658;
    c2 = -0.01427088;
    break;
  case 3:
    c0 = 0.84249538;
    c1 = 0.07684185;
    c2 = -0.00536879;
    break;
  case 4:
    c0 = 0.86140782;
    c1 = 0.07144126;
    c2 = -0.00558652;
    break;
  case 5:
    c0 = 0.87457740;
    c1 = 0.06578694;
    c2 = -0.00650447;
    break;
  case 6:
    c0 = 0.88438797;
    c1 = 0.06074265;
    c2 = -0.00736405;
    break;
  case 7:
    c0 = 0.89216620;
    c1 = 0.05669236;
    c2 = -0.00791222;
    break;
  case 8:
    c0 = 0.89874983;
    c1 = 0.05361696;
    c2 = -0.00815301;
    break;
  case 9:
    c0 = 0.90460032;
    c1 = 0.05167952;
    c2 = -0.00807893;
    break;
  case 10:
    c0 = 0.91034430;
    c1 = 0.05130753;
    c2 = -0.00746192;
    break;
  case 11:
    c0 = 0.91587675;
    c1 = 0.05180436;
    c2 = -0.00670711;
    break;
  case 12:
    c0 = 0.92121875;
    c1 = 0.05273801;
    c2 = -0.00588351;
    break;
  case 13:
    c0 = 0.92638195;
    c1 = 0.05400764;
    c2 = -0.00508452;
    break;
  case 14:
    c0 = 0.93123555;
    c1 = 0.05516163;
    c2 = -0.00437306;
    break;
  case 15:
    c0 = 0.93564993;
    c1 = 0.05596561;
    c2 = -0.00388152;
    break;
  case 16:
    c0 = 0.93976742;
    c1 = 0.05662274;
    c2 = -0.00348280;
    break;
  case 17:
    c0 = 0.94351703;
    c1 = 0.05694120;
    c2 = -0.00318821;
    break;
  case 18:
    c0 = 0.94557273;
    c1 = 0.05227591;
    c2 = -0.00400676;
    break;
  case 19:
    c0 = 0.95001614;
    c1 = 0.05681641;
    c2 = -0.00300628;
    break;
  case 20:
    c0 = 0.95281708;
    c1 = 0.05637607;
    c2 = -0.00304790;
    break;
  case 21:
    c0 = 0.95536256;
    c1 = 0.05575880;
    c2 = -0.00312988;
    break;
  case 22:
    c0 = 0.95754206;
    c1 = 0.05426060;
    c2 = -0.00385945;
    break;
  default:
    c0 =  0.056873 * logf( _m + 1e-3f ) + 0.781388;
    c1 =  0.05426f;
    c2 = -0.00386f;
  }

  float b = logf( _beta );
  float rho_hat = c0 + c1 * b + c2 * b * b;

  // ensure estimate is in [0,1]
  if( rho_hat < 0.0f ) {
    rho_hat = 0.0f;
  } else if( rho_hat > 1.0f ) {
    rho_hat = 1.0f;
  }

  return rho_hat;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Design frequency-shifted root-Nyquist filter based on
// the Kaiser-windowed sinc.
//
//  _k      :   filter over-sampling rate (samples/symbol)
//  _m      :   filter delay (symbols)
//  _beta   :   filter excess bandwidth factor (0,1)
//  _dt     :   filter fractional sample delay
//  _h      :   resulting filter [size: 2*_k*_m+1]
//  _rho    :   transition bandwidth adjustment, 0 < _rho < 1
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void liquid_firdes_rkaiser_bisection( unsigned int _k,
                                      unsigned int _m,
                                      float _beta,
                                      float _dt,
                                      float *_h,
                                      float *_rho )
{
  // algorithm:
  //  1. choose three initial points [x0, x1, x2] where x0 < x1 < x2
  //  2. choose xa = 0.5*(x0 + x1), bisection of [x0 x1]
  //  3. choose xb = 0.5*(x1 + x2), bisection of [x1 x2]
  //  4. x0 < xa < x1 < xb < x2
  //  5. evaluate points to obtain [y0, ya, y1, yb, y2]
  //  6. find minimum of three midpoints [ya, y1, yb]
  //  7. update initial points [x0, x1, x2] and go to step 2

  unsigned int i;

  unsigned int n = 2 * _k * _m + 1; // filter length

  // compute bandwidth adjustment estimate
  float rho_hat = rkaiser_approximate_rho( _m, _beta );

  // bandwidth adjustment
  float x0 = 0.5f * rho_hat;  // lower bound, old: rho_hat*0.9f;
  float x1 = rho_hat;         // midpoint: use initial estimate
  float x2 = 1.0f;            // upper bound, old: 1.0f - 0.9f*(1.0f-rho_hat);
  //x1 = 0.5f*(x0 + x2);      // bisect [x0,x1]

  // evaluate performance (ISI) of each bandwidth adjustment
  float y0 = liquid_firdes_rkaiser_internal_isi( _k, _m, _beta, _dt, x0, _h );
  float y1 = liquid_firdes_rkaiser_internal_isi( _k, _m, _beta, _dt, x1, _h );
  float y2 = liquid_firdes_rkaiser_internal_isi( _k, _m, _beta, _dt, x2, _h );

  // run parabolic search to find bandwidth adjustment x_hat which
  // minimizes the inter-symbol interference of the filter
  unsigned int p, pmax = 14;
  float x_hat = rho_hat;
  float xa, xb;
  float ya, yb;

  for( p = 0; p < pmax; p++ ) {

    // choose midway points xa, xb and compute ISI
    xa = 0.5f * ( x0 + x1 ); // bisect [x0,x1]
    xb = 0.5f * ( x1 + x2 ); // bisect [x1,x2]
    ya = liquid_firdes_rkaiser_internal_isi( _k, _m, _beta, _dt, xa, _h );
    yb = liquid_firdes_rkaiser_internal_isi( _k, _m, _beta, _dt, xb, _h );

    // find the minimum of [ya,y1,yb], update bounds
    if( y1 < ya && y1 < yb ) {
      x0 = xa;
      y0 = ya;
      x2 = xb;
      y2 = yb;
    } else if( ya < yb ) {
      x2 = x1;
      y2 = y1;
      x1 = xa;
      y1 = ya;
    } else {
      x0 = x1;
      y0 = y1;
      x1 = xb;
      y1 = yb;
    }

    x_hat = x1;
  };

  // re-design filter with optimal value for rho
  liquid_firdes_rkaiser_internal_isi( _k, _m, _beta, _dt, x_hat, _h );

  // normalize filter magnitude
  float e2 = 0.0f;
  for( i = 0; i < n; i++ ) e2 += _h[i] * _h[i];
  for( i = 0; i < n; i++ ) _h[i] *= sqrtf( _k / e2 );

  // save trasition bandwidth adjustment
  *_rho = x_hat;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Design frequency-shifted root-Nyquist filter based on
// the Kaiser-windowed sinc using the quadratic search method.
//
//  _k      :   filter over-sampling rate (samples/symbol)
//  _m      :   filter delay (symbols)
//  _beta   :   filter excess bandwidth factor (0,1)
//  _dt     :   filter fractional sample delay
//  _h      :   resulting filter [size: 2*_k*_m+1]
//  _rho    :   transition bandwidth adjustment, 0 < _rho < 1
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void liquid_firdes_rkaiser_quadratic( unsigned int _k,
                                      unsigned int _m,
                                      float _beta,
                                      float _dt,
                                      float *_h,
                                      float *_rho )
{

  // algorithm:
  //  1. choose initial bounding points [x0,x2] where x0 < x2
  //  2. choose x1 as bisection of [x0,x2]: x1 = 0.5*(x0+x2)
  //  3. choose x_hat as solution to quadratic equation (x0,y0), (x1,y1), (x2,y2)
  //  4. re-select boundary: (x0,y0) <- (x1,y1)   if x_hat > x1
  //                         (x2,y2) <- (x1,y1)   otherwise
  //  5. go to step 2

  unsigned int i;

  unsigned int n = 2 * _k * _m + 1; // filter length

  // compute bandwidth adjustment estimate
  float rho_hat = rkaiser_approximate_rho( _m, _beta );
  float rho_opt = rho_hat;

  // bandwidth adjustment
  float x0;           // lower bound
  float x1 = rho_hat; // initial estimate
  float x2;           // upper bound

  // evaluate performance (ISI) of each bandwidth adjustment
  float y0;
  float y1;
  float y2;
  float y_opt = 0.0f;

  // run parabolic search to find bandwidth adjustment x_hat which
  // minimizes the inter-symbol interference of the filter
  unsigned int p, pmax = 14;
  float x_hat = rho_hat;
  float dx  = 0.2f;       // bounding size
  float del = 0.0f;       // computed step size
  float tol = 1e-6f;      // tolerance
  for( p = 0; p < pmax; p++ ) {
    // choose boundary points
    x0 = x1 - dx;
    x2 = x1 + dx;

    // ensure boundary points are valid
    if( x0 <= 0.0f ) x0 = 0.01f;
    if( x2 >= 1.0f ) x2 = 0.99f;

    // evaluate all points
    y0 = liquid_firdes_rkaiser_internal_isi( _k, _m, _beta, _dt, x0, _h );
    y1 = liquid_firdes_rkaiser_internal_isi( _k, _m, _beta, _dt, x1, _h );
    y2 = liquid_firdes_rkaiser_internal_isi( _k, _m, _beta, _dt, x2, _h );

    // save optimum
    if( p == 0 || y1 < y_opt ) {
      rho_opt = x1;
      y_opt   = y1;
    }

    // compute minimum of quadratic function
    double ta = y0 * ( x1 * x1 - x2 * x2 ) +
                y1 * ( x2 * x2 - x0 * x0 ) +
                y2 * ( x0 * x0 - x1 * x1 );

    double tb = y0 * ( x1 - x2 ) +
                y1 * ( x2 - x0 ) +
                y2 * ( x0 - x1 );

    // update estimate
    x_hat = 0.5f * ta / tb;

    // ensure x_hat is within boundary (this will fail if y1 > y0 || y1 > y2)
    if( x_hat < x0 || x_hat > x2 ) {
      //fprintf(stderr,"warning: liquid_firdes_rkaiser_quadratic(), quadratic minimum outside boundary\n");
      break;
    }

    // break if step size is sufficiently small
    del = x_hat - x1;
    if( p > 3 && fabsf( del ) < tol )
      break;

    // update estimate, reduce bound
    x1 = x_hat;
    dx *= 0.5f;
  };

  // re-design filter with optimal value for rho
  liquid_firdes_rkaiser_internal_isi( _k, _m, _beta, _dt, rho_opt, _h );

  // normalize filter magnitude
  float e2 = 0.0f;
  for( i = 0; i < n; i++ ) e2 += _h[i] * _h[i];
  for( i = 0; i < n; i++ ) _h[i] *= sqrtf( _k / e2 );

  // save trasition bandwidth adjustment
  *_rho = rho_opt;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// compute filter coefficients and determine resulting ISI
//
//  _k      :   filter over-sampling rate (samples/symbol)
//  _m      :   filter delay (symbols)
//  _beta   :   filter excess bandwidth factor (0,1)
//  _dt     :   filter fractional sample delay
//  _rho    :   transition bandwidth adjustment, 0 < _rho < 1
//  _h      :   filter buffer [size: 2*_k*_m+1]
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
float liquid_firdes_rkaiser_internal_isi( unsigned int _k,
    unsigned int _m,
    float _beta,
    float _dt,
    float _rho,
    float *_h )
{

  unsigned int n = 2 * _k * _m + 1;           // filter length
  float kf = ( float )_k;                     // samples/symbol (float)
  float del = _beta * _rho / kf;              // transition bandwidth
  float As = estimate_req_filter_As( del, n ); // stop-band suppression
  float fc = 0.5f * ( 1 + _beta * ( 1.0f - _rho ) ) / kf; // filter cutoff

  // evaluate performance (ISI)
  float isi_max;
  float isi_rms;

  // compute filter
  liquid_firdes_kaiser( n, fc, As, _dt, _h );

  // compute filter ISI
  liquid_filter_isi( _h, _k, _m, &isi_rms, &isi_max );

  // return RMS of ISI
  return isi_rms;
}


