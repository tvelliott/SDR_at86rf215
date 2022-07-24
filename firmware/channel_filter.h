
//MIT License
//
//Copyright (c) 2018 tvelliott
//
//Permission is hereby granted, free of charge, to any person obtaining a copy
//of this software and associated documentation files (the "Software"), to deal
//in the Software without restriction, including without limitation the rights
//to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//copies of the Software, and to permit persons to whom the Software is
//furnished to do so, subject to the following conditions:
//
//The above copyright notice and this permission notice shall be included in all
//copies or substantial portions of the Software.
//
//THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
//SOFTWARE.




#ifndef __CHAN_FILTER_H__
#define __CHAN_FILTER_H__

#define ARM_MATH_CM4
#include <stdint.h>
#include <float.h>
#include <complex.h>
#include "arm_math.h"
#include "arm_const_structs.h"  //DSP includes


#define M_DEC 2 //decimate by.  Can be 2 -to- 32  @ 400khz Fs  for channel width of 12.5khz to 200 Khz
#define M_INTP 2 //interpolate by.  
#define SAMPLES_PER_FILTER_RUN ((NSAMPLES / M_DEC)*2)
#define NSAMPLES 256
#define NUM_TAPS 64
#define NUM_TAPS_INTP 32

extern float32_t iout[256];
extern float32_t qout[256];
void do_filter_decimate( float complex *iqdata );
void do_fircomp_decimate( float *in_data, float *out_data, int len );

#endif
