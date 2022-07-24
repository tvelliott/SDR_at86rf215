/* This file was automatically generated.  Do not edit! */
// internal structure
#ifndef __SYMSYNC_H__
#define __SYMSYNC_H__

#include "symsync_config.h"

#include "symsync_rf_arm32.h"
#include "firpfb_rf_arm32.h"
#include "iirfiltsos_rrrf_arm32.h"
#include "firdes_arm32.h"
#include "rrcos_arm32.h"
#include "window_rf_arm32.h"

struct _symsync_internal {
	struct firpfb_rf_s _mf_internal;
	struct firpfb_rf_s _dmf_internal;
	struct iirfiltsos_rrrf_s _iirflt_internal;

	struct window_rf_s _window_hf;
	struct window_rf_s _window_df;

	struct dotprod_rf_s _dprod_hf[SYMSYNC_NFILTERS]; //32 filter banks (defined in symsync_config.h)
	struct dotprod_rf_s _dprod_df[SYMSYNC_NFILTERS]; //32 filter banks (defined in symsync_config.h)
};

struct symsync_s {
  unsigned int h_len;         // matched filter length
  unsigned int k;             // samples/symbol (input)
  unsigned int k_out;         // samples/symbol (output)

  unsigned int decim_counter; // decimation counter
  int is_locked;              // synchronizer locked flag

  float rate;                 // internal resampling rate
  float del;                  // fractional delay step

  // floating-point timing phase
  float tau;                  // accumulated timing phase (0 <= tau <= 1)
  float tau_decim;            // timing phase, retained for get_tau() method
  float bf;                   // soft filterbank index
  int   b;                    // filterbank index

  // loop filter
  float q;                    // instantaneous timing error
  float q_hat;                // filtered timing error
  float B[3];                 // loop filter feed-forward coefficients
  float A[3];                 // loop filter feed-back coefficients
  struct iirfiltsos_rrrf_s *pll;        // loop filter object (iir filter)
  float rate_adjustment;      // internal rate adjustment factor

  unsigned int npfb;          // number of filters in the bank
  struct firpfb_rf_s *mf;           // matched filter
  struct firpfb_rf_s *dmf;           // derivative matched filter

};
void symsync_execute( struct symsync_s *_q, float *_x, unsigned int _nx, float *_y, unsigned int *_ny );
float symsync_get_tau( struct symsync_s *_q );
void symsync_lock( struct symsync_s *_q );
struct symsync_s *symsync_create_kaiser( struct symsync_s *q, unsigned int _k, unsigned int _m, float _beta, unsigned int _M );
struct symsync_s *symsync_create_rnyquist( struct symsync_s *q, int _type, unsigned int _k, unsigned int _m, float _beta, unsigned int _M );
void symsync_unlock( struct symsync_s *_q );
void symsync_set_lf_bw( struct symsync_s *_q, float _bt );
void symsync_reset( struct symsync_s *_q );
void symsync_set_output_rate( struct symsync_s *_q, unsigned int _k_out );
struct symsync_s *symsync_create( struct symsync_s *q, unsigned int _k, unsigned int _M, float *_h, unsigned int _h_len );
void symsync_output_debug_file( struct symsync_s *_q, const char *_filename );
void symsync_advance_internal_loop( struct symsync_s *_q, float _mf, float _dmf );
void symsync_step( struct symsync_s *_q, float _x, float *_y, unsigned int *_ny );

#endif
