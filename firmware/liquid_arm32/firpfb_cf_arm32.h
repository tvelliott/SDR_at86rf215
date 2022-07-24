/* This file was automatically generated.  Do not edit! */
#ifndef __FIRPFB_CF_H__
#define __FIRPFB_CF_H__

#include "resamp_config.h"

#include <float.h>
#include <complex.h>

#include "window_cf_arm32.h"
#include "dotprod_cf_arm32.h"

struct firpfb_cf_s {
  float complex *h;                      // filter coefficients array
  unsigned int h_len;         // total number of filter coefficients
  unsigned int h_sub_len;     // sub-sampled filter length
  unsigned int num_filters;   // number of filters

  struct window_cf_s *w;                 // window buffer
  struct dotprod_cf_s *dp[RESAMP_CF_NFILTERS];              // array of vector dot product objects
  float scale;                   // output scaling factor
};
void firpfb_cf_execute_block( struct firpfb_cf_s *_q, unsigned int _i, float complex *_x, unsigned int _n, float complex *_y );
void firpfb_cf_execute( struct firpfb_cf_s *_q, unsigned int _i, float complex *_y );
void firpfb_cf_push( struct firpfb_cf_s *_q, float complex _x );
void firpfb_cf_set_scale( struct firpfb_cf_s *_q, float _scale );
struct firpfb_cf_s *firpfb_cf_create_drnyquist( struct firpfb_cf_s *q, int _type, unsigned int _M, unsigned int _k, unsigned int _m, float _beta, struct window_cf_s *_window, struct dotprod_cf_s *_dp );
struct firpfb_cf_s *firpfb_cf_create_rnyquist( struct firpfb_cf_s *q, int _type, unsigned int _M, unsigned int _k, unsigned int _m, float _beta, struct window_cf_s *_window, struct dotprod_cf_s *_dp );
struct firpfb_cf_s *firpfb_cf_create_kaiser( struct firpfb_cf_s *q, unsigned int _M, unsigned int _m, float _fc, float _As, struct window_cf_s *_window, struct dotprod_cf_s *_dp );
void firpfb_cf_reset( struct firpfb_cf_s *_q );
struct firpfb_cf_s *firpfb_cf_create( struct firpfb_cf_s *q, unsigned int _M, float complex *_h, unsigned int _h_len, struct window_cf_s *_window, struct dotprod_cf_s *_dp );
#endif
