/* This file was automatically generated.  Do not edit! */
#ifndef __WINDOW_FUNCTIONS_LIQUID_H__
#define __WINDOW_FUNCTIONS_LIQUID_H__
float liquid_rcostaper_windowf(unsigned int _n,unsigned int _t,unsigned int _N);
float triangular(unsigned int _n,unsigned int _N,unsigned int _L);
float flattop(unsigned int _n,unsigned int _N);
float blackmanharris7(unsigned int _n,unsigned int _N);
float blackmanharris(unsigned int _n,unsigned int _N);
float hann(unsigned int _n,unsigned int _N);
float hamming(unsigned int _n,unsigned int _N);
void liquid_kbd_window(unsigned int _n,float _beta,float *_w);
float kaiser(unsigned int _n,unsigned int _N,float _beta,float _mu);
float liquid_kbd(unsigned int _n,unsigned int _N,float _beta);
#endif
