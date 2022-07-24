/* This file was automatically generated.  Do not edit! */
#ifndef __AUDIO_LEVELER_H__
#define __AUDIO_LEVELER_H__

#define LEVELER_FACTORS 6
void audio_leveler_execute(float *buffer,int len, float hf);
void init_leveler(void);
float leveler_adj(float val);

#endif
