/* This file was automatically generated.  Do not edit! */

#ifndef __RESAMP_RF_H__
#define __RESAMP_RF_H__

#include "resamp_config.h"
#include "firpfb_rf_arm32.h"

struct _resamp_rf_internal {
	struct firpfb_rf_s _firpfb_rf_internal;
	struct window_rf_s _window;
	struct dotprod_rf_s _dprod[RESAMP_RF_NFILTERS]; //32 filter banks (defined in resamp_config.h)
};

// defined:
//  float complex          output data type
//  float complex          coefficient data type
//  float complex          input data type
//  struct resamp_rf_s *     name-mangling macro
//  FIRPFB()    firpfb macro

struct resamp_rf_s {
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
    float y0;              // filterbank output at index b
    float y1;              // filterbank output at index b+1

    // polyphase filterbank properties/object
    unsigned int npfb;  // number of filters in the bank
    struct firpfb_rf_s *f;         // filterbank object (interpolator)

    enum {
        RESAMP_RF_STATE_BOUNDARY, // boundary between input samples
        RESAMP_RF_STATE_INTERP,   // regular interpolation
    } state;
};

void resamp_rf_execute_block(struct resamp_rf_s *_q,float *_x,unsigned int _nx,float *_y,unsigned int *_ny);
void resamp_rf_update_timing_state(struct resamp_rf_s *_q);
void resamp_rf_execute(struct resamp_rf_s *_q,float _x,float *_y,unsigned int *_num_written);
void resamp_rf_adjust_timing_phase(struct resamp_rf_s *_q,float _delta);
void resamp_rf_set_timing_phase(struct resamp_rf_s *_q,float _tau);
void resamp_rf_adjust_rate(struct resamp_rf_s *_q,float _gamma);
float resamp_rf_get_rate(struct resamp_rf_s *_q);
unsigned int resamp_rf_get_delay(struct resamp_rf_s *_q);
void resamp_rf_reset(struct resamp_rf_s *_q);
void resamp_rf_set_rate(struct resamp_rf_s *_q,float _rate);
struct resamp_rf_s *resamp_rf_create(struct resamp_rf_s *q,float _rate,unsigned int _m,float _fc,float _As,unsigned int _npfb);
extern struct firpfb_rf_s _firpfb_rf_internal;

#endif
