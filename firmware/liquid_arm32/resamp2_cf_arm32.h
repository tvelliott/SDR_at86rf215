/* This file was automatically generated.  Do not edit! */
#ifndef __RESAMP2_CF_H__
#define __RESAMP2_CF_H__

#include <float.h>
#include <complex.h>
#include <dotprod_cf_arm32.h>
#include <window_cf_arm32.h>

struct resamp2_s {
    float complex * h;                 // filter prototype
    unsigned int m;         // primitive filter length
    unsigned int h_len;     // actual filter length: h_len = 4*m+1
    float f0;               // center frequency [-1.0 <= f0 <= 1.0]
    float As;               // stop-band attenuation [dB]

    // filter component
    float complex * h1;                // filter branch coefficients
    struct dotprod_cf_s *dp;           // inner dot product object
    unsigned int h1_len;    // filter length (2*m)

    // input buffers
    struct window_cf_s *w0;            // input buffer (even samples)
    struct window_cf_s *w1;            // input buffer (odd samples)

    // halfband filter operation
    unsigned int toggle;
};

void resamp2_interp_execute(struct resamp2_s *_q,float complex _x,float complex *_y);
void resamp2_decim_execute(struct resamp2_s *_q,float complex *_x,float complex *_y);
void resamp2_synthesizer_execute(struct resamp2_s *_q,float complex *_x,float complex *_y);
void resamp2_analyzer_execute(struct resamp2_s *_q,float complex *_x,float complex *_y);
void resamp2_filter_execute(struct resamp2_s *_q,float complex _x,float complex *_y0,float complex *_y1);
unsigned int resamp2_get_delay(struct resamp2_s *_q);
void resamp2_reset(struct resamp2_s *_q);
struct resamp2_s *resamp2_create(struct resamp2_s *q, unsigned int _m,float _f0,float _As);
#endif
