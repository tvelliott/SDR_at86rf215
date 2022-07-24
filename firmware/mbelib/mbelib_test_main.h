/* This file was automatically generated.  Do not edit! */
#ifndef __MBELIB_TEST_H__
#define __MBELIB_TEST_H__
#include "dsd.h"

unsigned char imb_fgetc(dsd_opts *opts);
void mbelib_test_init(void);
void playSynthesizedVoice(dsd_opts *opts,dsd_state *state);
int mbe_processAudio(float *in_f,short *out_s);
int readImbe4400Data(dsd_opts *opts,dsd_state *state,char *imbe_d);
void mbe_test_tick();
extern int dsd_sample_count;
extern int as_count;
extern short dsd_sbuf[1024];
extern float dsd_fbuf[1024];
extern char t_imbe_d[88];
extern dsd_opts *t_opts;
extern dsd_opts dsdopts;
extern dsd_state *t_state;
extern dsd_state dsdstate;
extern int imb_idx;
#endif
