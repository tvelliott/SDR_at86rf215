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

//2018-10-12 t.elliott  hacked up liquid-dsp version for less object-oriented code

//
// Frequency demodulator
//

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <complex.h>

#include "globals.h"
#include "freqdem_cfrf_arm32.h"

#define TWO_PI ((float) (2.0f * M_PI))

static float kf;   // modulation index
static float ref;  // 1/(2*pi*kf)
static float complex r_prime; // previous received sample

    // initialize states
static float alpha         =  1.0f;   // phase adjustment factor
static float phase_in      =  0.0f;    // carrier phase offset
static float beta;
static float phase_out     = 0.0f;            // output signal phase
static float frequency_out = 0.0f;            // output signal frequency
float complex signal_out;
float c;
float s;
float phase_error;



///////////////////////////////////////////////////////////////////////////////
// create freqdem object
//  _kf     :   modulation factor. must be >0.0f && <=1.0
///////////////////////////////////////////////////////////////////////////////
void freqdem_init( float _kf )
{
  // set internal modulation factor
  kf = _kf;

  beta = 0.005*alpha*alpha; // frequency adjustment factor

  #ifndef INVERT_EXT_TUNER
    // compute derived values
    //ref = 1.0f / ( 2 * M_PI * kf );
    ref = 1.0f / ( 2 * M_PI * kf ) * 0.5;
  #else
    // compute derived values, for high-side LO (invert sign of output)
    ref = (1.0f / ( 2 * M_PI * kf )) * -1.0 * 0.5;
  #endif

  // reset modem object
  freqdem_reset();


}

///////////////////////////////////////////////////////////////////////////////
// reset modem object
///////////////////////////////////////////////////////////////////////////////
void freqdem_reset()
{
  // clear complex phase term
  r_prime = 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
float cargf_fast(float complex val) {
	return atan2f_fast( cimag(val), creal(val) );
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
float atan2f_fast( float y, float x )
{
	#if 1		//freqdemod_block of 1000,   1.546 x faster, no errors 

  //http://pubs.opengroup.org/onlinepubs/009695399/functions/atan2.html
  //Volkan SALMA

  const float ONEQTR_PI = M_PI / 4.0;
  const float THRQTR_PI = 3.0 * M_PI / 4.0;
  float r, angle;
  float abs_y = fabs( y ) + 1e-10f;    // kludge to prevent 0/0 condition
  if( x < 0.0f ) {
    r = ( x + abs_y ) / ( abs_y - x );
    angle = THRQTR_PI;
  } else {
    r = ( x - abs_y ) / ( x + abs_y );
    angle = ONEQTR_PI;
  }
  angle += ( 0.1963f * r * r - 0.9817f ) * r;
  if( y < 0.0f )
    return( -angle );     // negate if in quad III or IV
  else
    return( angle );
	#endif

	#if 0		//freqdemod_block of 1000,   1.19 x faster, more errors

	#define PI_FLOAT     3.14159265f
	#define PIBY2_FLOAT  1.5707963f

		if( x == 0.0f ) {
			if( y > 0.0f ) return PIBY2_FLOAT;
			if( y == 0.0f ) return 0.0f;
			return -PIBY2_FLOAT;
		}
		float atan;
		float z = y / x;
		if( fabs( z ) < 1.0f ) {
			atan = z / ( 1.0f + 0.28f * z * z );
			if( x < 0.0f ) {
				if( y < 0.0f ) return atan - PI_FLOAT;
				return atan + PI_FLOAT;
			}
		} else {
			atan = PIBY2_FLOAT - z / ( z * z + 0.28f );
			if( y < 0.0f ) return atan - PI_FLOAT;
		}
		return atan;

	#endif

}

///////////////////////////////////////////////////////////////////////////////
// demodulate block of samples
//  _q      :   frequency demodulator object
//  _r      :   received signal r(t) [size: _n x 1]
//  _n      :   number of input, output samples
//  _m      :   message signal m(t), [size: _n x 1]
///////////////////////////////////////////////////////////////////////////////
void freqdem_demodulate( float complex *_r, int _n, float *_m )
{
	#if 0
  unsigned int i;

  for( i = 0; i < _n; i++ ) {
    _m[i] = cargf( conjf( r_prime ) * _r[i] ) * ref;
    r_prime = _r[i];
  }
	#endif

	freqdem_demodulate_fast( _r, _n, _m );
}

///////////////////////////////////////////////////////////////////////////////
// demodulate block of samples
//  _q      :   frequency demodulator object
//  _r      :   received signal r(t) [size: _n x 1]
//  _n      :   number of input, output samples
//  _m      :   message signal m(t), [size: _n x 1]
///////////////////////////////////////////////////////////////////////////////
void freqdem_demodulate_fast( float complex *_r, int _n, float *_m )
{
  unsigned int i;

  for( i = 0; i < _n; i++ ) {
    _m[i] = cargf_fast( conjf( r_prime ) * _r[i] ) * ref;
    r_prime = _r[i];
  }
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//float normalize_angle(float theta) {
  //float normalized = theta % TWO_PI;
  //normalized = (normalized + TWO_PI) % TWO_PI; 
 // return normalized;
//}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void freqdem_demodulate_fast_pll( float complex *_r, int _n, float *_m )
{
  unsigned int i;

  for( i = 0; i < _n; i++ ) {

    //cexp(I * z) = ccos(z) + I * csin(z)
    //signal_out = cexpf(_Complex_I * phase_out);   //too slow
    //signal_out = ccos(phase_out) + _Complex_I * csin(phase_out);    //too slow


    //while(phase_out>TWO_PI) phase_out -= TWO_PI; 
    //while(phase_out<0.0) phase_out += TWO_PI; 

    signal_out = cosf(phase_out) + _Complex_I * sinf(phase_out);    //works
    //signal_out = arm_cos_f32(phase_out) + _Complex_I * arm_sin_f32(phase_out);


    // compute phase error estimate
    phase_error = cargf_fast( _r[i] * conjf(signal_out) );
    _m[i] = phase_error * ref;
    
    // apply loop filter and correct output phase and frequency
    //phase_out     += alpha * phase_error;    // adjust phase    not necessary when alpha=1.0
    phase_out     += phase_error;    // adjust phase
    //frequency_out +=  beta * phase_error;    // adjust frequency
    
    // increment input and output phase values
    //phase_out += frequency_out;
    //

  }
}
