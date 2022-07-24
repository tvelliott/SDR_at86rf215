/* This file was automatically generated.  Do not edit! */
#ifndef __P25_DECODE_H__
#define __P25_DECODE_H__

#include "symsync_rf_arm32.h"

//3877.932617, mn:-3306.478516, ct:285.727112, std:2113.062012

#define DEFAULT_CENTER 0.0f
#define DEFAULT_MIN -4000.0f
#define DEFAULT_MAX 4000.0f
#define DEFAULT_STD_DEV 2000.0f;

#define DEFAULT_STD_DEV_FACTOR 1.7f
#define DEFAULT_STATS_ALPHA 0.005f
#define DEFAULT_ABS_MAX_ERROR 4500 
#define DEFAULT_HISTORY_SIZE 150 

#define OUT_BUFFER_SIZE 16384 //must be power of 2

void Golay23_Correct(unsigned int *block);
inline unsigned int Golay23_CorrectAndGetErrCount(unsigned int *block){
  unsigned int i, errs = 0, in = *block;

  Golay23_Correct (block);

  in >>= 11;
  in ^= *block;
  for (i = 0; i < 12; i++) {
      if ((in >> i) & 1) {
          errs++;
      }
  }

  return (errs);
};
unsigned int hdu_correct_hex_word(unsigned char *hex_and_parity,unsigned int codeword_bits);
void pack_data(char *in,char *out,int errors);
int p25_mbe_eccImbe7200x4400Data(char imbe_fr[8][23],char *imbe_d);
void p25_mbe_demodulateImbe7200x4400Data(char imbe[8][23]);
int p25_mbe_eccImbe7200x4400C0(char imbe_fr[8][23]);
void deinterleave_frame(int dibit_n,uint8_t dibit_val);
void p25_processHDU(char *hex_and_parity);
float p25_decode(float sample_f, struct symsync_s *q);
char *get_talkgroup_str(int talkgroup);
int p25_is_synced(void);
int p25_working_state(void);
void set_channel_timeout_zero(void);

extern volatile short out_buffer[OUT_BUFFER_SIZE];
extern volatile short upsampled[160 * 6];

#endif
