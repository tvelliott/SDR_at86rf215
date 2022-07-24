/* This file was automatically generated.  Do not edit! */

#ifndef __RESAMP_CF_H__
#define __RESAMP_CF_H__

#include <complex.h> 
#include "resamp_config.h"
#include "firpfb_cf_arm32.h"
#include "dotprod_cf_arm32.h"

struct _resamp_cf_internal {
	struct firpfb_cf_s _firpfb_cf_internal;
	struct window_cf_s _window;
	struct dotprod_cf_s _dprod[RESAMP_CF_NFILTERS]; //32 filter banks (defined in resamp_config.h)
	float _hf[RESAMP_CF_HLEN];
	float complex _h[RESAMP_CF_HLEN];
};

// defined:
//  float complex          output data type
//  float complex          coefficient data type
//  float complex          input data type
//  struct resamp_cf_s *     name-mangling macro
//  FIRPFB()    firpfb macro

struct resamp_cf_s {
    // filter design parameters
    unsigned int m;     // filter semi-length, h_len = 2*m + 1
    float As;           // filter stop-band attenuation
    float fc;           // filter cutoff frequency

    // resampling properties/states
    float rate;         // resampling rate (ouput/input)
    float del;          // fractional delay step

    // floating-point phase
    float tau;          // accumulated timing phase, 0 <= tau < 1
    float bf;           // soft filterbank index, bf = tau*npfb = b + mu
    int b;              // base filterbank index, 0 <= b < npfb
    float mu;           // fractional filterbank interpolation value, 0 <= mu < 1
    float complex y0;              // filterbank output at index b
    float complex y1;              // filterbank output at index b+1

    // polyphase filterbank properties/object
    unsigned int npfb;  // number of filters in the bank
    struct firpfb_cf_s *f;         // filterbank object (interpolator)

    enum {
        RESAMP_CF_STATE_BOUNDARY, // boundary between input samples
        RESAMP_CF_STATE_INTERP,   // regular interpolation
    } state;
};

void resamp_cf_execute_block(struct resamp_cf_s *_q,float complex *_x,unsigned int _nx,float complex *_y,unsigned int *_ny);
void resamp_cf_update_timing_state(struct resamp_cf_s *_q);
void resamp_cf_execute(struct resamp_cf_s *_q,float complex _x,float complex *_y,unsigned int *_num_written);
void resamp_cf_adjust_timing_phase(struct resamp_cf_s *_q,float _delta);
void resamp_cf_set_timing_phase(struct resamp_cf_s *_q,float _tau);
void resamp_cf_adjust_rate(struct resamp_cf_s *_q,float _gamma);
float resamp_cf_get_rate(struct resamp_cf_s *_q);
unsigned int resamp_cf_get_delay(struct resamp_cf_s *_q);
void resamp_cf_reset(struct resamp_cf_s *_q);
void resamp_cf_set_rate(struct resamp_cf_s *_q,float _rate);
struct resamp_cf_s *resamp_cf_create(struct resamp_cf_s *q,float _rate,unsigned int _m,float _fc,float _As,unsigned int _npfb);
extern struct firpfb_cf_s _firpfb_cf_internal;

#endif
