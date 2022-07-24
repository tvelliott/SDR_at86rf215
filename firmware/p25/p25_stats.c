#include <stdio.h>
#include <math.h>
#include "p25_stats.h"

static float *data_ptr = NULL;
static int data_len = 0;

///////////////////////////////////////////////////
///////////////////////////////////////////////////
void p25_stats_init( float *fptr, int len)
{
  data_ptr = fptr;
  data_len = len;
}


///////////////////////////////////////////////////
///////////////////////////////////////////////////
float p25_stats_max()
{
  float max = -999999.0f;
  int i;
  for(i=0; i<data_len; i++) {
    if(data_ptr[i]>max) max = data_ptr[i];
  }
  return max;
}
///////////////////////////////////////////////////
///////////////////////////////////////////////////
float p25_stats_min()
{
  float min = 999999.0f;
  int i;
  for(i=0; i<data_len; i++) {
    if(data_ptr[i]<min) min = data_ptr[i];
  }
  return min;
}

///////////////////////////////////////////////////
///////////////////////////////////////////////////
float p25_stats_mean()
{
  float sum = 0.0f;
  int i;
  for(i=0;i<data_len;i++) {
    sum += data_ptr[i];
  }

  return sum/data_len;
}

///////////////////////////////////////////////////
///////////////////////////////////////////////////
float p25_stats_rms()
{
  float sum = 0.0f;
  int i;
  for(i=0;i<data_len;i++) {
    sum += data_ptr[i]*data_ptr[i];
  }

  return sqrtf(sum)/data_len;
}
///////////////////////////////////////////////////
///////////////////////////////////////////////////
float p25_stats_mean_deviation()
{
  float mean = p25_stats_mean();
  float temp = 0.0f;
  float a;
  int i;

  for(i=0;i<data_len;i++) {
    a = data_ptr[i];
    temp += fabs( (mean-a) );
  }

  return temp/data_len; 
}

///////////////////////////////////////////////////
///////////////////////////////////////////////////
float p25_stats_variance()
{
  float mean = p25_stats_mean();
  float temp = 0.0f;
  float a;
  int i;

  for(i=0;i<data_len;i++) {
    a = data_ptr[i];
    temp += (mean-a) * (mean-a);
  }

  return temp/data_len; 
}
///////////////////////////////////////////////////
///////////////////////////////////////////////////
float p25_stats_stddev()
{
  float v = p25_stats_variance();
  return sqrt(v); 
}
