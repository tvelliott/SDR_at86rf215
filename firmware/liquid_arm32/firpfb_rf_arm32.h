/* This file was automatically generated.  Do not edit! */
#ifndef __FIRPFB_RF_H__
#define __FIRPFB_RF_H__

#include "symsync_config.h"


#include <float.h>
#include <complex.h>

#include "window_rf_arm32.h"
#include "dotprod_rf_arm32.h"

struct firpfb_rf_s {
  float *h;                      // filter coefficients array
  unsigned int h_len;         // total number of filter coefficients
  unsigned int h_sub_len;     // sub-sampled filter length
  unsigned int num_filters;   // number of filters

  struct window_rf_s *w;                 // window buffer
  struct dotprod_rf_s *dp[SYMSYNC_NFILTERS];              // array of vector dot product objects
  float scale;                   // output scaling factor
};
void firpfb_rf_execute_block( struct firpfb_rf_s *_q, unsigned int _i, float *_x, unsigned int _n, float *_y );
void firpfb_rf_execute( struct firpfb_rf_s *_q, unsigned int _i, float *_y );
void firpfb_rf_push( struct firpfb_rf_s *_q, float _x );
void firpfb_rf_set_scale( struct firpfb_rf_s *_q, float _scale );
struct firpfb_rf_s *firpfb_rf_create_drnyquist( struct firpfb_rf_s *q, int _type, unsigned int _M, unsigned int _k, unsigned int _m, float _beta, struct window_rf_s *_window, struct dotprod_rf_s *_dp );
struct firpfb_rf_s *firpfb_rf_create_rnyquist( struct firpfb_rf_s *q, int _type, unsigned int _M, unsigned int _k, unsigned int _m, float _beta, struct window_rf_s *_window, struct dotprod_rf_s *_dp );
struct firpfb_rf_s *firpfb_rf_create_kaiser( struct firpfb_rf_s *q, unsigned int _M, unsigned int _m, float _fc, float _As, struct window_rf_s *_window, struct dotprod_rf_s *_dp );
void firpfb_rf_reset( struct firpfb_rf_s *_q );
struct firpfb_rf_s *firpfb_rf_create( struct firpfb_rf_s *q, unsigned int _M, float *_h, unsigned int _h_len, struct window_rf_s *_window, struct dotprod_rf_s *_dp );
#endif
