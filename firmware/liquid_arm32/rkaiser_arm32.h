/* This file was automatically generated.  Do not edit! */
#ifndef __RKAISER_H__
#define __RKAISER_H__
float liquid_firdes_rkaiser_internal_isi(unsigned int _k,unsigned int _m,float _beta,float _dt,float _rho,float *_h);
void liquid_firdes_rkaiser_bisection(unsigned int _k,unsigned int _m,float _beta,float _dt,float *_h,float *_rho);
float rkaiser_approximate_rho(unsigned int _m,float _beta);
void liquid_firdes_arkaiser(unsigned int _k,unsigned int _m,float _beta,float _dt,float *_h);
void liquid_firdes_rkaiser_quadratic(unsigned int _k,unsigned int _m,float _beta,float _dt,float *_h,float *_rho);
void liquid_firdes_rkaiser(unsigned int _k,unsigned int _m,float _beta,float _dt,float *_h);
#endif
