#ifndef __STATS_H__
#define __STATS_H__

#include <stdint.h>
#include <math.h>

void p25_stats_init(float *, int);
float p25_stats_rms(void);
float p25_stats_stddev(void);
float p25_stats_min(void);
float p25_stats_max(void);
float p25_stats_mean(void);
float p25_stats_variance(void);
float p25_stats_mean_deviation(void);

#endif
