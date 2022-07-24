/* This file was automatically generated.  Do not edit! */

#ifndef __WINDOW_CF_H__
#define __WINDOW_CF_H__

#include <complex.h> 
#include "symsync_config.h"


struct window_cf_s {
  //float complex v[SYMSYNC_WINDOW_V_SIZE];                       // allocated array pointer
  float complex *v;                       // allocated array pointer
  unsigned int len;           // length of window
  unsigned int m;             // floor(log2(len)) + 1
  unsigned int n;             // 2^m
  unsigned int mask;          // n-1
  unsigned int num_allocated; // number of elements allocated
  // in memory
  unsigned int read_index;
};
void window_cf_write( struct window_cf_s *_q, float complex *_v, unsigned int _n );
void window_cf_push( struct window_cf_s *_q, float complex _v );
void window_cf_index( struct window_cf_s *_q, unsigned int _i, float complex *_v );
void window_cf_read( struct window_cf_s *_q, float complex **_v );
void window_cf_reset( struct window_cf_s *_q );
struct window_cf_s *window_cf_create( struct window_cf_s *q, unsigned int _n );

#endif
