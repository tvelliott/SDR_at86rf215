/* This file was automatically generated.  Do not edit! */
#ifndef __AMPMODEM_H__
#define __AMPMODEM_H__

#include "firhilb_arm32.h"
#include "nco_cf_arm32.h"

enum {
    LIQUID_AMPMODEM_DSB=0,  // double side-band
    LIQUID_AMPMODEM_USB,    // single side-band (upper)
    LIQUID_AMPMODEM_LSB     // single side-band (lower)
};

struct ampmodem_s {
    float m;                    // modulation index
    int type;  // modulation type
    int suppressed_carrier;     // suppressed carrier flag
    float fc;                   // carrier frequency

    // demod objects
    struct nco_cf_s *oscillator;

    // suppressed carrier
    // TODO : replace DC bias removal with iir filter object
    float ssb_alpha;    // dc bias removal
    float ssb_q_hat;

    // single side-band
    struct firhilb_s *hilbert;   // hilbert transform

};

void ampmodem_demodulate_block(struct ampmodem_s *_q,float complex *_r,unsigned int _n,float *_m);
void ampmodem_demodulate(struct ampmodem_s *_q,float complex _y,float *_x);
void ampmodem_modulate_block(struct ampmodem_s *_q,float *_m,unsigned int _n,float complex *_s);
void ampmodem_modulate(struct ampmodem_s *_q,float _x,float complex *_y);
void ampmodem_reset(struct ampmodem_s *_q);
struct ampmodem_s * ampmodem_create(float _m,float _fc,int _type,int _suppressed_carrier);
#endif
