/* This file was automatically generated.  Do not edit! */
#ifndef __WDELAY_RF_H__
#define __WDELAY_RF_H__

#include <math.h>
#include <complex.h>

struct wdelay_rf_s {
    float * v;                      // allocated array pointer
    unsigned int delay;         // length of window
    unsigned int read_index;
};
void wdelay_rf_push(struct wdelay_rf_s *_q,float _v);
void wdelay_rf_read(struct wdelay_rf_s *_q,float *_v);
void wdelay_rf_reset(struct wdelay_rf_s *_q);
struct wdelay_rf_s *wdelay_rf_create(unsigned int _delay);

#endif
