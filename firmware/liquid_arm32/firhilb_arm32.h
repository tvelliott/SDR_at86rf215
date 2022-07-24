/* This file was automatically generated.  Do not edit! */

#ifndef __FIRHILB_H__
#define __FIRHILB_H__

#include <complex.h>

#include "window_rf_arm32.h"
#include "dotprod_rf_arm32.h"

struct firhilb_s {
    float * h;                  // filter coefficients
    float complex * hc;         // filter coefficients (complex)
    unsigned int h_len;     // length of filter
    float As;               // filter stop-band attenuation [dB]

    unsigned int m;         // filter semi-length, h_len = 4*m+1

    // quadrature filter component
    float * hq;                 // quadrature filter coefficients
    unsigned int hq_len;    // quadrature filter length (2*m)

    // input buffers
    struct window_rf_s *w0;            // input buffer (even samples)
    struct window_rf_s *w1;            // input buffer (odd samples)

    // vector dot product
    struct dotprod_rf_s *dpq;

    // regular real-to-complex/complex-to-real operation
    unsigned int toggle;
};

void firhilb_interp_execute_block(struct firhilb_s *_q,float complex *_x,unsigned int _n,float *_y);
void firhilb_interp_execute(struct firhilb_s *_q,float complex _x,float *_y);
void firhilb_decim_execute_block(struct firhilb_s *_q,float *_x,unsigned int _n,float complex *_y);
void firhilb_decim_execute(struct firhilb_s *_q,float *_x,float complex *_y);
void firhilb_c2r_execute(struct firhilb_s *_q,float complex _x,float *_y);
void firhilb_r2c_execute(struct firhilb_s *_q,float _x,float complex *_y);
void firhilb_reset(struct firhilb_s *_q);
struct firhilb_s *firhilb_create(unsigned int _m,float _As);

#endif
