/* This file was automatically generated.  Do not edit! */
// firfilt object structure
#ifndef __FIRFILT_RF_H__
#define __FIRFILT_RF_H__

#include <float.h>
#include <complex.h>
#include "dotprod_rf_arm32.h"

struct firfilt_rf_s {
    float * h;             // filter coefficients array [size; h_len x 1]
    unsigned int h_len; // filter length

    // use array as internal buffer (faster)
    float * w;                 // internal buffer object
    unsigned int w_len;     // window length
    unsigned int w_mask;    // window index mask
    unsigned int w_index;   // window read index
    struct dotprod_rf_s *dp;           // dot product object
    float scale;               // output scaling factor
};

float firfilt_rf_groupdelay(struct firfilt_rf_s *_q,float _fc);
void firfilt_rf_freqresponse(struct firfilt_rf_s *_q,float _fc,float *_H);
unsigned int firfilt_rf_get_length(struct firfilt_rf_s *_q);
void firfilt_rf_execute_block(struct firfilt_rf_s *_q,float *_x,unsigned int _n,float *_y);
void firfilt_rf_execute(struct firfilt_rf_s *_q,float *_y);
void firfilt_rf_push(struct firfilt_rf_s *_q,float _x);
void firfilt_rf_get_scale(struct firfilt_rf_s *_q,float *_scale);
void firfilt_rf_set_scale(struct firfilt_rf_s *_q,float _scale);
struct firfilt_rf_s *firfilt_rf_create_rect(struct firfilt_rf_s *q,unsigned int _n);
struct firfilt_rf_s *firfilt_rf_create_rnyquist(struct firfilt_rf_s *q,int _type,unsigned int _k,unsigned int _m,float _beta,float _mu);
struct firfilt_rf_s *firfilt_rf_create_kaiser(struct firfilt_rf_s *q,unsigned int _n,float _fc,float _As,float _mu);
void firfilt_rf_reset(struct firfilt_rf_s *_q);
struct firfilt_rf_s *firfilt_rf_create(struct firfilt_rf_s *q,float *_h,unsigned int _n);
#endif
