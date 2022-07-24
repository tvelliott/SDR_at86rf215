/* This file was automatically generated.  Do not edit! */

#ifndef __DOTPROD_CF_H__
#define __DOTPROD_CF_H__

#include <complex.h>

// portable structured dot product object
struct dotprod_cf_s {
  //float complex h[MAX_DOTPROD_HLEN];              // coefficients array
  float complex *h;              // coefficients array
  unsigned int n;     // length
};
void dotprod_cf_execute( struct dotprod_cf_s *_q, float complex *_x, float complex *_y );
struct dotprod_cf_s *dotprod_cf_create( struct dotprod_cf_s *q, float complex *_h, unsigned int _n );
//void dotprod_cf_run4( float complex *_h, float complex *_x, unsigned int _n, float complex *_y );
//void dotprod_cf_run( float complex *_h, float complex *_x, unsigned int _n, float complex *_y );
#endif
