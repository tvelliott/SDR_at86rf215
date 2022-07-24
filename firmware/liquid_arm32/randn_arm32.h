/* This file was automatically generated.  Do not edit! */
#ifndef __RAND_ARM32_H__
#define __RAND_ARM32_H__

#define randf_inline() ((float) rand() / (float) RAND_MAX)

// generate x ~ Gamma(delta,1)
float randgammaf_delta(float _delta);
float randf_cdf(float _x);
float randf_pdf(float _x);
float randnf_cdf(float _x,float _eta,float _sig);
float randnf_pdf(float _x,float _eta,float _sig);
void cawgn(float complex *_x,float _nstd);
float complex icrandnf();
void crandnf(float complex *_y);
void awgn(float *_x,float _nstd);
float randf();
float randnf();
#endif
