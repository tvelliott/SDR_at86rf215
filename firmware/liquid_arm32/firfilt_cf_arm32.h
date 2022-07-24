/* This file was automatically generated.  Do not edit! */
// firfilt object structure
#ifndef __FIRFILT_CF_H__
#define __FIRFILT_CF_H__

#include <float.h>
#include <complex.h>
#include "dotprod_cf_arm32.h"

struct firfilt_cf_s {
    float complex *h;             // filter coefficients array [size; h_len x 1]
    unsigned int h_len; // filter length

    // use array as internal buffer (faster)
    float complex *w;                 // internal buffer object
    unsigned int w_len;     // window length
    unsigned int w_mask;    // window index mask
    unsigned int w_index;   // window read index
    struct dotprod_cf_s *dp;           // dot product object
    float scale;               // output scaling factor
};

void firfilt_cf_freqresponse(struct firfilt_cf_s *_q,float _fc,float complex *_H);
unsigned int firfilt_cf_get_length(struct firfilt_cf_s *_q);
void firfilt_cf_execute_block(struct firfilt_cf_s *_q, float complex *_x, unsigned int _n, float complex *_y);
void firfilt_cf_execute(struct firfilt_cf_s *_q, float complex *_y);
void firfilt_cf_push(struct firfilt_cf_s *_q, float complex _x);
void firfilt_cf_get_scale(struct firfilt_cf_s *_q,float *_scale);
void firfilt_cf_set_scale(struct firfilt_cf_s *_q,float _scale);
struct firfilt_cf_s *firfilt_cf_create_rect(struct firfilt_cf_s *q,unsigned int _n);
struct firfilt_cf_s *firfilt_cf_create_rnyquist(struct firfilt_cf_s *q, int _type, unsigned int _k, unsigned int _m, float _beta, float _mu);
struct firfilt_cf_s *firfilt_cf_create_kaiser(struct firfilt_cf_s *q, unsigned int _n, float _fc, float _As, float _mu);
void firfilt_cf_reset(struct firfilt_cf_s *_q);
struct firfilt_cf_s *firfilt_cf_create(struct firfilt_cf_s *q,float complex *_h,unsigned int _n);
#endif
