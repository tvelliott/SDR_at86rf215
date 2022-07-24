
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



#include "p25_stats.h"
#include "fft.h"

#define ARM_MATH_CM7 1
#include "arm_common_tables.h"
#include "arm_math.h"

#define SAMPLES     128
#define FFT_SIZE    (SAMPLES / 2)

float32_t Input[SAMPLES];
float32_t Output[FFT_SIZE];
arm_cfft_radix2_instance_f32 fftS;    //ARM CFFT module

static float current_hf = 0.1f;

/////////////////////////////////////////////////////
/////////////////////////////////////////////////////
void fft_shift( float32_t *data, int len )
{
  float32_t tmp[SAMPLES];
  int i;
  if( len > SAMPLES ) len = SAMPLES;

  for( i = 0; i < len / 2; i++ ) {
    tmp[i] = data[i + len / 2];
  }
  for( i = 0; i < len / 2; i++ ) {
    tmp[i + len / 2] = data[i];
  }
  for( i = 0; i < len; i++ ) {
    data[i] = tmp[i];
  }
}


/////////////////////////////////////////////////////
/////////////////////////////////////////////////////
float do_fft( float *buffer, int len )
{
  int i;

  for( i = 0; i < SAMPLES; i++ ) {
    Input[i] = ( float32_t ) buffer[i];
  }

  //mult by fft blackman harris window
  //arm_mult_f32( Input, fft_win_128, Input, FFT_SIZE*2);

  //initialize FFT  intflag=0  bitreverse=1
  arm_cfft_radix2_init_f32( &fftS, FFT_SIZE, 0, 1 );
  //do FFT_SIZE point complex fft 32-bit
  arm_cfft_radix2_f32( &fftS,  Input );

  //calculate magnitude of fft for each bin
  arm_cmplx_mag_f32( Input, Output, FFT_SIZE );


  //not needed for this application
  //fft_shift(Output, FFT_SIZE/2);
  //printf("\r\n");
  //for(i=48;i<64;i++) {
  //  printf("%3.1f,", Output[i]);
  //}

  p25_stats_init( &Output[48], FFT_SIZE / 4 );
  float hf_mean = p25_stats_mean();
  float val = ( 1.0 - hf_mean / 3500.0f ) * 0.25f;

  if( val < 0.0 ) val = 0.0;

  if( val > current_hf ) current_hf += 0.02f;
  if( val < current_hf ) current_hf -= 0.02f;
  if( current_hf < 0 ) current_hf = 0.0f;
  if( current_hf > 0.25f ) current_hf = 0.25f;

  //printf("\r\n%3.4f", current_hf);

  return current_hf;
}
