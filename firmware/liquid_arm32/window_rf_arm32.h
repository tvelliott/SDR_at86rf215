/* This file was automatically generated.  Do not edit! */

#ifndef __WINDOW_RF_H__
#define __WINDOW_RF_H__

#include "symsync_config.h"


struct window_rf_s {
  //float v[SYMSYNC_WINDOW_V_SIZE];                       // allocated array pointer
  float *v;                       // allocated array pointer
  unsigned int len;           // length of window
  unsigned int m;             // floor(log2(len)) + 1
  unsigned int n;             // 2^m
  unsigned int mask;          // n-1
  unsigned int num_allocated; // number of elements allocated
  // in memory
  unsigned int read_index;
};
void window_rf_write( struct window_rf_s *_q, float *_v, unsigned int _n );
void window_rf_push( struct window_rf_s *_q, float _v );
void window_rf_index( struct window_rf_s *_q, unsigned int _i, float *_v );
void window_rf_read( struct window_rf_s *_q, float **_v );
void window_rf_reset( struct window_rf_s *_q );
struct window_rf_s *window_rf_create( struct window_rf_s *q, unsigned int _n );

#endif
