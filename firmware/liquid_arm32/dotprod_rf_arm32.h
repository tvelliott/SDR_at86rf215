/* This file was automatically generated.  Do not edit! */

#ifndef __DOTPROD_RF_H__
#define __DOTPROD_RF_H__

// portable structured dot product object
struct dotprod_rf_s {
  //float h[MAX_DOTPROD_HLEN];              // coefficients array
  float *h;              // coefficients array
  unsigned int n;     // length
};
void dotprod_rf_execute( struct dotprod_rf_s *_q, float *_x, float *_y );
struct dotprod_rf_s *dotprod_rf_create( struct dotprod_rf_s *q, float *_h, unsigned int _n );
//void dotprod_rf_run4( float *_h, float *_x, unsigned int _n, float *_y );
//void dotprod_rf_run( float *_h, float *_x, unsigned int _n, float *_y );
#endif
