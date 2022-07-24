/* This file was automatically generated.  Do not edit! */
#ifndef __IIRFILTSOS_RRRF_H__
#define __IIRFILTSOS_RRRF_H__

#include "symsync_config.h"


struct iirfiltsos_rrrf_s {
  float b[3];    // feed-forward coefficients
  float a[3];    // feed-back coefficients

  // internal buffering
  float x[3];    // Direct form I  buffer (input)
  float y[3];    // Direct form I  buffer (output)
  float v[3];    // Direct form II buffer

};

float iirfiltsos_rrrf_groupdelay( struct iirfiltsos_rrrf_s *_q, float _fc );
void iirfiltsos_rrrf_execute_df1( struct iirfiltsos_rrrf_s *_q, float _x, float *_y );
void iirfiltsos_rrrf_execute_df2( struct iirfiltsos_rrrf_s *_q, float _x, float *_y );
void iirfiltsos_rrrf_execute( struct iirfiltsos_rrrf_s *_q, float _x, float *_y );
void iirfiltsos_rrrf_reset( struct iirfiltsos_rrrf_s *_q );
void iirfiltsos_rrrf_set_coefficients( struct iirfiltsos_rrrf_s *_q, float *_b, float *_a );
struct iirfiltsos_rrrf_s *iirfiltsos_rrrf_create( struct iirfiltsos_rrrf_s *q, float *_b, float *_a );

#endif
