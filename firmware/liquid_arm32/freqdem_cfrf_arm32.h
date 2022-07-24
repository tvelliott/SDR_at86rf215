/* This file was automatically generated.  Do not edit! */
#ifndef __FREQDEM_H__
#define __FREQDEM_H__

#include "symsync_config.h"

float cargf_fast(float complex val);
float atan2f_fast( float y, float x );
void freqdem_demodulate( float complex *_r, int _n, float *_m );
void freqdem_demodulate_fast( float complex *_r, int _n, float *_m );
void freqdem_reset();
void freqdem_init( float _kf );
void freqdem_demodulate_fast_pll( float complex *_r, int _n, float *_m );
#endif
