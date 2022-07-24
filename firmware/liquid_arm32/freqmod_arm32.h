/* This file was automatically generated.  Do not edit! */
// freqmod
#ifndef __FREQMOD_H__
#define __FREQMOD_H__

struct freqmod_s {
    float kf;   // modulation factor for FM
    float ref;  // phase reference: kf*2^16

    // look-up table
    unsigned int sincos_table_len;      // table length: 10 bits
    uint16_t     sincos_table_phase;    // accumulated phase: 16 bits
    float complex *sincos_table;          // sin|cos look-up table: 2^10 entries
};

void freqmod_modulate_block(struct freqmod_s *_q,float *_m,unsigned int _n,float complex *_s);
void freqmod_modulate(struct freqmod_s *_q,float _m,float complex *_s);
void freqmod_reset(struct freqmod_s *_q);
struct freqmod_s *freqmod_create(float _kf);

#endif
