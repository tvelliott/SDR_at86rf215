/* This file was automatically generated.  Do not edit! */
#ifndef __WDELAY_CF_H__
#define __WDELAY_CF_H__

struct wdelay_cf_s {
    float complex * v;                      // allocated array pointer
    unsigned int delay;         // length of window
    unsigned int read_index;
};

void wdelay_cf_push(struct wdelay_cf_s *_q,float complex _v);
void wdelay_cf_read(struct wdelay_cf_s *_q,float complex *_v);
void wdelay_cf_reset(struct wdelay_cf_s *_q);
struct wdelay_cf_s *wdelay_cf_create(unsigned int _delay);
#endif
