
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





#include <math.h>
#include <float.h>
#include <complex.h>
#include "arm_math.h"
#include "channel_filter.h"


//decimation

static float32_t fir_dec_state_f32_I_8[NSAMPLES + 128 - 1];
static float32_t fir_dec_state_f32_Q_8[NSAMPLES + 128 - 1];
static arm_fir_decimate_instance_f32 Si_dec_8;
static arm_fir_decimate_instance_f32 Sq_dec_8;

static float32_t fir_dec_state_f32_I_32[NSAMPLES + 128 - 1];
static float32_t fir_dec_state_f32_Q_32[NSAMPLES + 128 - 1];
static arm_fir_decimate_instance_f32 Si_dec_32;
static arm_fir_decimate_instance_f32 Sq_dec_32;

static float32_t dc_state_f32_I[NSAMPLES + 134 - 1];
static float32_t dc_state_f32_Q[NSAMPLES + 134 - 1];
static arm_fir_instance_f32 dc_i;
static arm_fir_instance_f32 dc_q;

static float32_t fir_dec_state_f32_i_stage1[NSAMPLES + 128 - 1];
static float32_t fir_dec_state_f32_q_stage1[NSAMPLES + 128 - 1];
static arm_fir_decimate_instance_f32 fir_i_stage1;
static arm_fir_decimate_instance_f32 fir_q_stage1;

static float32_t fir_dec_state_f32_i_stage2[NSAMPLES + 128 - 1];
static float32_t fir_dec_state_f32_q_stage2[NSAMPLES + 128 - 1];
static arm_fir_decimate_instance_f32 fir_i_stage2;
static arm_fir_decimate_instance_f32 fir_q_stage2;

float32_t iout[256];
float32_t qout[256];

float dc_removal_63[] = {
  0.001040582726326743, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -0.002081165452653486, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0.001040582726326743
};

//./basic_fir_filter -d1 -b0.24 -l128 -c2 -w4 -g0.7 -a2
//./basic_fir_filter -d1 -b0.25 -l128 -c2 -w4 -g0.7 -a2
float fir_comp_filter[] = {
  -0.00000, -0.00000, 0.00000, 0.00000, -0.00001, -0.00001, 0.00001, 0.00002, -0.00002, -0.00003, 0.00004, 0.00005, -0.00007, -0.00009, 0.00011, 0.00014, -0.00017
    , -0.00021, 0.00026, 0.00031, -0.00037, -0.00044, 0.00053, 0.00062, -0.00073, -0.00086, 0.00100, 0.00116, -0.00135, -0.00155, 0.00178, 0.00204, -0.00233
    , -0.00265, 0.00300, 0.00340, -0.00384, -0.00432, 0.00485, 0.00544, -0.00609, -0.00681, 0.00760, 0.00847, -0.00944, -0.01051, 0.01170, 0.01302, -0.01451
    , -0.01619, 0.01809, 0.02027, -0.02279, -0.02574, 0.02926, 0.03353, -0.03886, -0.04572, 0.05494, 0.06809, -0.08852, -0.12496, 0.20942, 0.63000, 0.63000
    , 0.20942, -0.12496, -0.08852, 0.06809, 0.05494, -0.04572, -0.03886, 0.03353, 0.02925, -0.02574, -0.02279, 0.02027, 0.01809, -0.01619, -0.01451, 0.01302
    , 0.01170, -0.01051, -0.00944, 0.00847, 0.00760, -0.00681, -0.00609, 0.00544, 0.00485, -0.00432, -0.00384, 0.00340, 0.00300, -0.00265, -0.00233, 0.00204
    , 0.00178, -0.00155, -0.00135, 0.00116, 0.00100, -0.00086, -0.00073, 0.00062, 0.00053, -0.00044, -0.00037, 0.00031, 0.00026, -0.00021, -0.00017, 0.00014
    , 0.00011, -0.00009, -0.00007, 0.00005, 0.00004, -0.00003, -0.00002, 0.00002, 0.00001, -0.00001, -0.00001, 0.00000, 0.00000, -0.00000, -0.00000
  };



// ./basic_fir_filter -d1 -b0.8 -l256 -c2 -w4 -g0.7 -a2
//
const float32_t channel_filter[] = {
  -0.00001, -0.00001, -0.00001, -0.00000, 0.00001, 0.00003, 0.00007, 0.00013, 0.00022, 0.00034, 0.00051, 0.00071, 0.00096, 0.00125, 0.00157, 0.00191, 0.00224
    , 0.00253, 0.00275, 0.00284, 0.00274, 0.00240, 0.00174, 0.00069, -0.00081, -0.00284, -0.00542, -0.00860, -0.01236, -0.01669, -0.02149, -0.02665, -0.03201
    , -0.03733, -0.04235, -0.04673, -0.05011, -0.05207, -0.05218, -0.04997, -0.04500, -0.03684, -0.02508, -0.00940, 0.01047, 0.03469, 0.06330, 0.09624, 0.13328
    , 0.17408, 0.21813, 0.26480, 0.31333, 0.36287, 0.41245, 0.46108, 0.50770, 0.55127, 0.59080, 0.62535, 0.65407, 0.67627, 0.69138, 0.69904, 0.69904
    , 0.69138, 0.67627, 0.65407, 0.62535, 0.59080, 0.55127, 0.50770, 0.46108, 0.41245, 0.36287, 0.31333, 0.26480, 0.21813, 0.17408, 0.13328, 0.09624
    , 0.06330, 0.03469, 0.01047, -0.00940, -0.02508, -0.03684, -0.04500, -0.04997, -0.05218, -0.05207, -0.05011, -0.04673, -0.04235, -0.03733, -0.03201, -0.02665
    , -0.02149, -0.01669, -0.01236, -0.00860, -0.00542, -0.00284, -0.00081, 0.00069, 0.00174, 0.00240, 0.00274, 0.00284, 0.00275, 0.00253, 0.00224, 0.00191
    , 0.00157, 0.00125, 0.00096, 0.00071, 0.00051, 0.00034, 0.00022, 0.00013, 0.00007, 0.00003, 0.00001, -0.00000, -0.00001, -0.00001, -0.00001
  };

const float32_t channel_filter_256[] = {
  -0.00000, 0.00000, 0.00000, -0.00000, -0.00000, 0.00000, 0.00000, 0.00000, -0.00000, -0.00000, 0.00000, 0.00000, 0.00000, -0.00000, -0.00000, 0.00000, 0.00000
    , 0.00000, -0.00000, -0.00001, 0.00000, 0.00001, 0.00001, -0.00001, -0.00001, -0.00000, 0.00002, 0.00001, -0.00001, -0.00003, 0.00000, 0.00003, 0.00002
    , -0.00002, -0.00004, 0.00000, 0.00005, 0.00004, -0.00004, -0.00007, -0.00000, 0.00008, 0.00006, -0.00006, -0.00011, 0.00000, 0.00013, 0.00009, -0.00009
    , -0.00016, 0.00000, 0.00019, 0.00013, -0.00014, -0.00024, -0.00000, 0.00028, 0.00018, -0.00020, -0.00034, 0.00000, 0.00039, 0.00026, -0.00028, -0.00048
    , 0.00000, 0.00055, 0.00036, -0.00038, -0.00066, -0.00000, 0.00074, 0.00049, -0.00052, -0.00089, 0.00000, 0.00100, 0.00065, -0.00069, -0.00118, 0.00000
    , 0.00132, 0.00087, -0.00091, -0.00156, -0.00000, 0.00174, 0.00114, -0.00120, -0.00205, 0.00000, 0.00228, 0.00148, -0.00157, -0.00267, 0.00000, 0.00298
    , 0.00194, -0.00205, -0.00350, -0.00000, 0.00392, 0.00256, -0.00271, -0.00466, 0.00000, 0.00525, 0.00345, -0.00368, -0.00636, -0.00000, 0.00731, 0.00486
    , -0.00525, -0.00921, -0.00000, 0.01100, 0.00751, -0.00835, -0.01520, 0.00000, 0.02008, 0.01473, -0.01806, -0.03768, -0.00000, 0.08823, 0.16370, 0.16370
    , 0.08823, 0.00000, -0.03768, -0.01806, 0.01473, 0.02008, 0.00000, -0.01520, -0.00835, 0.00751, 0.01100, 0.00000, -0.00921, -0.00525, 0.00486, 0.00731
    , 0.00000, -0.00636, -0.00368, 0.00345, 0.00525, 0.00000, -0.00466, -0.00271, 0.00256, 0.00392, 0.00000, -0.00350, -0.00205, 0.00194, 0.00298, 0.00000
    , -0.00267, -0.00157, 0.00148, 0.00228, 0.00000, -0.00205, -0.00120, 0.00114, 0.00174, 0.00000, -0.00156, -0.00091, 0.00087, 0.00132, 0.00000, -0.00118
    , -0.00069, 0.00065, 0.00100, 0.00000, -0.00089, -0.00052, 0.00049, 0.00074, -0.00000, -0.00066, -0.00038, 0.00036, 0.00055, 0.00000, -0.00048, -0.00028
    , 0.00026, 0.00039, 0.00000, -0.00034, -0.00020, 0.00018, 0.00028, -0.00000, -0.00024, -0.00014, 0.00013, 0.00019, 0.00000, -0.00016, -0.00009, 0.00009
    , 0.00013, 0.00000, -0.00011, -0.00006, 0.00006, 0.00008, -0.00000, -0.00007, -0.00004, 0.00004, 0.00005, 0.00000, -0.00004, -0.00002, 0.00002, 0.00003
    , 0.00000, -0.00003, -0.00001, 0.00001, 0.00002, -0.00000, -0.00001, -0.00001, 0.00001, 0.00001, 0.00000, -0.00001, -0.00000, 0.00000, 0.00000, 0.00000
    , -0.00000, -0.00000, 0.00000, 0.00000, 0.00000, -0.00000, -0.00000, 0.00000, 0.00000, 0.00000, -0.00000, -0.00000, 0.00000, 0.00000, -0.00000
  };

int8_t init_dc_removal = 0;
int8_t init_fir_dec_8 = 0;
int8_t init_fir_dec_32 = 0;
int8_t init_fir_stage1 = 0;
int8_t init_fir_stage2 = 0;

static float32_t input_i[NSAMPLES];
static float32_t input_q[NSAMPLES];

/////////////////////////////////////////////////////
/////////////////////////////////////////////////////
void cf_fft_shift( float32_t *data, int len )
{
  float32_t tmp[2048];
  int i;
  if( len > 2048 ) len = 2048;

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
//////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////
void do_fircomp_dec2_stage1( float complex *in_data )
{
  int i;

  if( !init_fir_stage1 ) {
    arm_fir_decimate_init_f32( &fir_i_stage1, 128, 2, &fir_comp_filter[0], &fir_dec_state_f32_i_stage1[0], 256 );
    arm_fir_decimate_init_f32( &fir_q_stage1, 128, 2, &fir_comp_filter[0], &fir_dec_state_f32_q_stage1[0], 256 );
    init_fir_stage1 = 1;
  }

  for( i = 0; i < 256; i++ ) {
    input_i[i] = ( float32_t ) creal( in_data[i] );
    input_q[i] = ( float32_t ) cimag( in_data[i] );
  }

  arm_fir_decimate_f32( &fir_i_stage1, input_i, iout, 256 );
  arm_fir_decimate_f32( &fir_q_stage1, input_q, qout, 256 );

}
//////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////
void do_fircomp_dec2_stage2( float complex *in_data )
{
  int i;

  if( !init_fir_stage2 ) {
    arm_fir_decimate_init_f32( &fir_i_stage2, 128, 2, &fir_comp_filter[0], &fir_dec_state_f32_i_stage2[0], 128 );
    arm_fir_decimate_init_f32( &fir_q_stage2, 128, 2, &fir_comp_filter[0], &fir_dec_state_f32_q_stage2[0], 128 );
    init_fir_stage2 = 1;
  }

  for( i = 0; i < 128; i++ ) {
    input_i[i] = ( float32_t ) creal( in_data[i] );
    input_q[i] = ( float32_t ) cimag( in_data[i] );
  }

  arm_fir_decimate_f32( &fir_i_stage2, input_i, iout, 128 );
  arm_fir_decimate_f32( &fir_q_stage2, input_q, qout, 128 );

}

//////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////
void do_filter_decimate_8( float complex *iqdata )
{

  int i;

  if( !init_fir_dec_8 ) {
    arm_fir_decimate_init_f32( &Si_dec_8, 128, 8, &channel_filter[0], &fir_dec_state_f32_I_8[0], NSAMPLES );
    arm_fir_decimate_init_f32( &Sq_dec_8, 128, 8, &channel_filter[0], &fir_dec_state_f32_Q_8[0], NSAMPLES );
    init_fir_dec_8 = 1;
  }


  //I filtering

  for( i = 0; i < NSAMPLES; i++ ) {
    input_i[i] = ( float32_t ) creal( iqdata[i] );
    input_q[i] = ( float32_t ) cimag( iqdata[i] );
  }
  arm_fir_decimate_f32( &Si_dec_8, input_i, iout, NSAMPLES );
  arm_fir_decimate_f32( &Sq_dec_8, input_q, qout, NSAMPLES );

}
//////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////
void do_filter_decimate_32( float complex *iqdata )
{

  int i;

  if( !init_fir_dec_32 ) {
    arm_fir_decimate_init_f32( &Si_dec_32, 128, 32, &channel_filter[0], &fir_dec_state_f32_I_32[0], NSAMPLES );
    arm_fir_decimate_init_f32( &Sq_dec_32, 128, 32, &channel_filter[0], &fir_dec_state_f32_Q_32[0], NSAMPLES );
    init_fir_dec_32 = 1;
  }


  //I filtering

  for( i = 0; i < NSAMPLES; i++ ) {
    input_i[i] = ( float32_t ) creal( iqdata[i] );
    input_q[i] = ( float32_t ) cimag( iqdata[i] );
  }
  arm_fir_decimate_f32( &Si_dec_32, input_i, iout, NSAMPLES );
  arm_fir_decimate_f32( &Sq_dec_32, input_q, qout, NSAMPLES );

}
//static float32_t dc_state_f32_I[NSAMPLES + 128 -1];
//static float32_t dc_state_f32_Q[NSAMPLES + 128 -1];
//static arm_fir_dc_instance_f32 dc_i;
//static arm_fir_dc_instance_f32 dc_q;
//////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////
float dc_removal( float complex *iqdata, float *ioff, float *qoff )
{

  int i;

  if( !init_dc_removal ) {
    arm_fir_init_f32( &dc_i, 63, &dc_removal_63[0], &dc_state_f32_I[0], NSAMPLES );
    arm_fir_init_f32( &dc_q, 63, &dc_removal_63[0], &dc_state_f32_Q[0], NSAMPLES );
    init_dc_removal = 1;
  }


  for( i = 0; i < NSAMPLES; i++ ) {
    input_i[i] = ( float32_t ) creal( iqdata[i] );
    input_q[i] = ( float32_t ) cimag( iqdata[i] );
  }
  arm_fir_f32( &dc_i, input_i, iout, NSAMPLES );
  arm_fir_f32( &dc_q, input_q, qout, NSAMPLES );

  float f1 = 0.0f;
  float f2 = 0.0f;
  for( i = 0; i < NSAMPLES; i++ ) {
    f1 += iout[i];
    f2 += qout[i];
  }
  *ioff = f1;
  *qoff = f2;

}
