/* This file was automatically generated.  Do not edit! */

#ifndef __FIRDES_H__
#define __FIRDES_H__

#include "symsync_config.h"

enum {
  LIQUID_FIRDESPM_BANDPASS = 0,   // regular band-pass filter
  LIQUID_FIRDESPM_DIFFERENTIATOR, // differentiating filter
  LIQUID_FIRDESPM_HILBERT         // Hilbert transform
};

// weighting type specifier
enum {
  LIQUID_FIRDESPM_FLATWEIGHT = 0, // flat weighting
  LIQUID_FIRDESPM_EXPWEIGHT,      // exponential weighting
  LIQUID_FIRDESPM_LINWEIGHT,      // linear weighting
};

enum {
  LIQUID_FIRFILT_UNKNOWN = 0,
  LIQUID_FIRFILT_KAISER,
  LIQUID_FIRFILT_PM,
  LIQUID_FIRFILT_RCOS,
  LIQUID_FIRFILT_FEXP,
  LIQUID_FIRFILT_FSECH,
  LIQUID_FIRFILT_FARCSECH,
  LIQUID_FIRFILT_ARKAISER,
  LIQUID_FIRFILT_RKAISER,
  LIQUID_FIRFILT_RRC,
  LIQUID_FIRFILT_hM3,
  LIQUID_FIRFILT_GMSKTX,
  LIQUID_FIRFILT_GMSKRX,
  LIQUID_FIRFILT_RFEXP,
  LIQUID_FIRFILT_RFSECH,
  LIQUID_FIRFILT_RFARCSECH
};
int liquid_getopt_str2firfilt( const char *_str );
void liquid_filter_isi( float *_h, unsigned int _k, unsigned int _m, float *_rms, float *_max );
float liquid_filter_crosscorr( float *_h, unsigned int _h_len, float *_g, unsigned int _g_len, int _lag );
float liquid_filter_autocorr( float *_h, unsigned int _h_len, int _lag );
void liquid_firdes_doppler( unsigned int _n, float _fd, float _K, float _theta, float *_h );
void liquid_firdes_prototype( int _type, unsigned int _k, unsigned int _m, float _beta, float _dt, float *_h );
void liquid_firdes_kaiser( unsigned int _n, float _fc, float _As, float _mu, float *_h );
float kaiser_beta_As( float _As );
float estimate_req_filter_len_Kaiser( float _df, float _As );
float estimate_req_filter_df( float _As, unsigned int _N );
float estimate_req_filter_As( float _df, unsigned int _N );
float estimate_req_filter_len_Herrmann( float _df, float _As );
unsigned int estimate_req_filter_len( float _df, float _As );
#endif
