

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include <complex.h>

#include "command.h"
#include "globals.h"
#include "p25p1_const.h"
#include "Golay.h"

#include "mbelib/mbelib.h"
#include "p25_stats.h"
#include "p25_decode.h"
#include "p25p1_const.h"

#include "resamp_rf_arm32.h"
#include "resamp_cf_arm32.h"

#include "mbelib/mbelib_test_main.h"
#include "dsd.h"
#include "audio_leveler.h"
#include "fft.h"

extern volatile int agc_locked;

int do_mute=0;

int verbose_cnt;

static int symsync_reset_mod;

static int sync_count;
void MX_LWIP_Process(void);

void p25_reset_stats(void);
void udp_send_data(char *buffer, int len);
int upsample_audio(char *buffer, int len);
void p25_net_tick(void);

static char talkgroup_str[128];

extern int audio_frames_done;
extern int do_silent_frame;

static int debug = 0;

static int gerr;

static uint64_t p25_p1sync_inv = 0x00007757d7dd55dd;
static uint64_t p25_p1sync =     0x00005575f5ff77ff;
static uint64_t dreg;

static int is_synced;

static uint32_t nac;
static uint8_t duid;
static uint8_t prev_duid;

static int status_bit_counter;
static int bit_count;
static int is_inverse;
static int voice_state;

static char imbe_d[88];
static char imbe_fr[8][23];
//static char voice_data[12];
//static char p25_header[4]={'.','i','m','b'};

//header data
static char hdu_hex_data[9*8];    

static char ldu_hex_parity[20*6];


static int errors;
static int errors2;

static int dibit_skip;
static int found_header;


static int eq_state;

static int working_state;
static float working_center;
static float working_min;
static float working_max;

static float center=DEFAULT_CENTER;
static float min=DEFAULT_MIN;
static float max=DEFAULT_MAX;
static float umid;
static float lmid;
static float mean_deviation=DEFAULT_STD_DEV;
static float cval=0;

static float prev_sample_f;
static float dev_delta;
static float dev_err;

static float sample_history[DEFAULT_HISTORY_SIZE];

static unsigned char algid = 0;
static unsigned int mfid = 0;
static unsigned int talkgroup = 0;
static int word_count;
static int hex_bit_count;

volatile short out_buffer[OUT_BUFFER_SIZE];
volatile int out_s;
volatile int out_e;
volatile int out_cnt;
volatile int is_playing;

static float use_gain = 1.0;

static int last_err; 
static uint32_t stime;

static int p25_reset_tick;
struct symsync_s *ss_q=NULL;

extern struct resamp_rf_s *audio_resamp_q;
extern struct resamp_cf_s *ch_resamp_q;

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
void process_mbe(char *t_imbe_d, int err1, int err2, int enc, int is_last) {

	int i;

	t_state->errs = err1;
	t_state->errs2 = err2;

	//if(config->p25_logging>1) printf("\r\nerrors %d", err1+err2);

	t_opts->uvquality = 2;	//about 7 max for stm32,  2 is ok

  mbe_processImbe4400Dataf(dsd_fbuf, &t_state->errs, &t_state->errs2, t_state->err_str, t_imbe_d, t_state->cur_mp, t_state->prev_mp, t_state->prev_mp_enhanced, t_opts->uvquality);

	//etime = HAL_GetTick();
	//if(etime-stime >= 5) if(config->p25_logging>1) printf("\r\nLDU proc time %d ms", (int) (etime-stime) );


	if(enc==0x00) { //non-encrypted voice

		if( err1==0 && err2==0 && last_err==0) {


			p25_stats_init(dsd_fbuf, 160);
			float max = p25_stats_max();
			float gain = 4096.0 / max;

			if(gain > 2.5) gain = 2.5;
			else if(gain < 1.0/5.0) gain = 0.0; 

			if(gain > use_gain) use_gain += 0.04 * gain;
			if(gain < use_gain) use_gain -= 0.04 * gain;

		}

		last_err = err1+err2;

		//if(config->p25_logging>1) printf("\r\nmax %3.1f, gain %3.1f", max, gain);

    for(i=0;i<160;i++) {
      dsd_fbuf[i] *= use_gain*0.25;
    }

    float hf = do_fft((float *) &dsd_fbuf[16], 128);  //analyze higher frequencies for hf-boost

    //normalize to 1.0
    for(i=0;i<160;i++) {
      dsd_fbuf[i] /= 32768.0f;
    }
    //compress/limit/hf-boost audio
    audio_leveler_execute(dsd_fbuf, 160, hf);
    //expand
    for(i=0;i<160;i++) {
      dsd_fbuf[i] *= 30000.0f*config->audio_volume_f;
    }
		mbe_processAudio(dsd_fbuf, dsd_sbuf);  //convert float to shorts ... 160 of them.  Output is 8 kHz
		audio_frames_done++;

		char *ptr = (char *) dsd_sbuf;

    if(out_s==out_e) {
      resamp_rf_reset(audio_resamp_q);	//why does this need to be done?
    }

    if( (out_e^out_s) >= 1024) is_playing=1;

    //re-sample the audio from 8 KHz to 16.28 KHz to match desired audio hardware/clocks
    int n = upsample_audio((char *) ptr, 320);

    for(i=0;i<n;i++) {
      out_buffer[out_e++] = upsampled[i];
      out_e &= OUT_BUFFER_SIZE-1;
    }
	}

}

/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////
void p25_net_tick() {

	if(p25_reset_tick++ > 15000) {
		p25_reset_tick=0;
		p25_reset_stats();
	}
}
/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////
int p25_working_state() {
	return eq_state;
}

/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////
int p25_is_synced() {
	return is_synced;
}

/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////
void p25_reset_stats() {

    if(ss_q==NULL) return;

    set_channel_timeout_zero();

    if(uptime_sec < 20) return;

		if(config->p25_logging>3) printf("\r\np25_reset_stats");

		is_synced=0;
		nac = 0;
		duid = -1;
		status_bit_counter=0;
		bit_count=0;
		is_inverse=0;
		voice_state=-1;
		working_state=0;

		max = DEFAULT_MAX; 
		min = DEFAULT_MIN; 
		center = DEFAULT_CENTER; 
		mean_deviation=DEFAULT_STD_DEV;

		algid = 0;
		//talkgroup=0;
		found_header=0;

		working_max = 0.0f;

		symsync_reset(ss_q);
		resamp_cf_reset(ch_resamp_q);
}

/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////
void p25_zeros() {
	char zeros[320];
	memset(zeros,0x00,sizeof(zeros));
	//if(out_cnt==0) udp_send_data(zeros,40);
}

/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////
float p25_decode(float sample_f, struct symsync_s *q ) {


	int i;
	float n_min;
	float n_max;
	float n_center;
	float n_mean_deviation;
	float stats_alpha = DEFAULT_STATS_ALPHA; 

	ss_q = q;

  /*
	if(working_max==0.0) {
		stats_alpha = 0.002f;	//fix the levels quicly during header
	}
	else {
		stats_alpha = 0.00001f;
	}
  */

	//sanity check
	if( sample_f > DEFAULT_ABS_MAX_ERROR ) sample_f = DEFAULT_ABS_MAX_ERROR; 
	if( sample_f < -DEFAULT_ABS_MAX_ERROR ) sample_f = -DEFAULT_ABS_MAX_ERROR; 

	//if( !isnormal(sample_f) ) sample_f=prev_sample_f;

	//dc offset correction
	//d_avg = d_avg*d_beta+sample_f*d_alpha;
	//sample_f -= d_avg;


	///////////////////////////////////////////////////////////////////////////////////////
	//  use filtered sample history + stats to find correct levels for the slicer
	///////////////////////////////////////////////////////////////////////////////////////
	
	//update match buffer with new sample
	for(i=1;i<DEFAULT_HISTORY_SIZE;i++) {
		sample_history[i-1] = sample_history[i]; 
	}
	sample_history[DEFAULT_HISTORY_SIZE-1]= sample_f; 


	p25_stats_init(sample_history, DEFAULT_HISTORY_SIZE);

	n_mean_deviation = p25_stats_mean_deviation();

	n_max = p25_stats_max();
	n_min = p25_stats_min();

	n_center = (n_max+n_min)/2.0; 

	dev_delta = fabs(mean_deviation - n_mean_deviation);
	dev_err = mean_deviation * 0.001f;

		center += stats_alpha*(n_center-center);
		mean_deviation += stats_alpha*(n_mean_deviation-mean_deviation);
		max = center+mean_deviation*DEFAULT_STD_DEV_FACTOR;
		min = center-mean_deviation*DEFAULT_STD_DEV_FACTOR;

	//detect no carrier and limit
	if( max > DEFAULT_ABS_MAX_ERROR  || min < -DEFAULT_ABS_MAX_ERROR ) {

		if(working_max!=0.0) {
			center = working_center;
			max = working_max;
			min = working_min;
		}
		else {
			
			max = DEFAULT_MAX; 
			min = DEFAULT_MIN; 
			center = DEFAULT_CENTER; 
			mean_deviation=DEFAULT_STD_DEV;
		}
	}

			max = DEFAULT_MAX; 
			min = DEFAULT_MIN; 
			center = DEFAULT_CENTER; 
			mean_deviation=DEFAULT_STD_DEV;


	//use max/min to define the center and slicer levels
	umid = center + ( fabs(max-center) * 0.55f);
	lmid = center - ( fabs(center-min) * 0.55f); 


  //if(verbose_cnt++%48000) printf("\r\n%f, %f, %f, %f", max, min, center, mean_deviation);

  prev_sample_f = sample_f;

	cval = 0; 

	////////////////////////////////////////////////////////////////
		dreg <<=2;

		if(!is_synced) {
			if(sample_f>umid) dreg |= 0x01; 
				else dreg |= 0x03;
			found_header=0;
		}
		else {
			if(sample_f>umid) {
				dreg |= 0x01; 
				cval = max;
			}
			else if(sample_f>center) {
				dreg |= 0x00; 
				cval = umid;
			}
			else if(sample_f<lmid) {
				dreg |= 0x03; 
				cval = min;
			}
			else { 
				dreg |= 0x02; 
				cval = lmid;
			}
		}

		
		//if(tau_mod++%100==0) if(config->p25_logging>1) printf("\r\ntau,%3.5f,%3.1f", symsync_get_tau(q), sample_f); 

		if(is_inverse) dreg ^= 0x02;
	////////////////////////////////////////////////////////////////


	dibit_skip=0;

	status_bit_counter++;

	if( status_bit_counter==37  ) {
		status_bit_counter=1;
		dibit_skip=1;
	}
	else { 
		bit_count+=2;
	}

			//else if(is_synced) if(config->p25_logging>1) printf("\r\nstatus bit");

	if( (dreg&0xffffffffffff) == p25_p1sync ) {
		if(config->p25_logging>2) printf("\r\nfound sync %d, freq %3.4f", sync_count++, get_current_freq());

		is_synced=(32*6+8);
		nac = 0;
		duid = -1;
		status_bit_counter=0;
		bit_count=0;
		is_inverse=0;
		voice_state=-1;
		working_state=0;
		p25_reset_tick=0;
		symsync_set_lf_bw( q, 0.005f );
    reset_channel_timeout();

	}
	else if( (dreg&0xffffffffffff) == p25_p1sync_inv ) {		//somebody swapped I/Q
		if(config->p25_logging>2) printf("\r\nfound sync_inv %d", sync_count++);
		//is_synced=(32*6+8);
		//nac = 0;
		//duid = -1;
		//status_bit_counter=0;
		//bit_count=0;
		//is_inverse=1;
		//voice_state=-1;
		//working_state=0;
	}


	if(bit_count > 426) {
		bit_count=0;
	}

	if(is_synced) {
		is_synced--;

		if(is_synced==0) {

			algid = 0;
			talkgroup=0;
			bit_count=0;
			voice_state=0;
			duid=-1;
			found_header=0;
			//if(out_fd!=0) close(out_fd);
			//out_fd =0;
			working_state=0;

      /*
			if(working_max!=0.0) {
				center = working_center;
				max = working_max;
				min = working_min;
			}
      */

			symsync_set_lf_bw( q, 0.01f );
      symsync_reset(ss_q);
      resamp_cf_reset(ch_resamp_q);

		}
	}

  if(!is_synced && symsync_reset_mod++%48000==0) {
			symsync_set_lf_bw( q, 0.01f );
      symsync_reset(ss_q);
      resamp_cf_reset(ch_resamp_q);
  }
  else if(is_synced) {
    symsync_reset_mod=1;
  }

	if(is_synced && !dibit_skip && voice_state==-1) {

			//12-bits, idx 1-6
		if(bit_count>=2 && bit_count<=12) {
			nac <<=2;
			nac |= (uint32_t) (dreg&0x03);

			duid=0;
		}

		if(bit_count>=14 && bit_count<=16) {

      if( (nac!=config->p25_network_id && !config->p25_network_id_allow_all)) { 
				bit_count=0;
				voice_state=-1;
				is_synced=0;
				talkgroup=0;
				algid=0;
				return 0;
			}

			duid <<=2;
			duid |= (uint8_t) (dreg&0x03);
			//if(bit_count==16) if(config->p25_logging>1) printf("\nnac: %06x, duid: %01x", nac, duid); 
		}

		if(bit_count==18) {
			voice_state=0;

			switch( (int) duid ) {


				case	0x00	:	
          if(!debug && config->p25_logging>1) printf("\r\n\n  ->HDU, Frequency %3.4f", get_current_freq()); 

					//if(out_fd!=0x00) {
				//		if(out_fd!=0) close(out_fd);
				//		out_fd=0;
				//	}

					found_header=1;
					word_count=0;
					memset((char *)hdu_hex_data,0x00,sizeof(hdu_hex_data));
					is_synced=1200;
					if(working_state==0) working_state++;
					algid=0x00;
					mfid=0x00;
					use_gain = 0.5;
					//flush_audio();
				break;

				case	0x05	:	
					is_synced=1200;

					if(talkgroup!=0 && !debug && config->p25_logging>1) {
            printf("\r\n  ->LDU1 (VOICE) %s, freq: %3.4f, rssi: %d, ch: %d", get_talkgroup_str(talkgroup),  get_current_freq(), get_rssi(), get_current_channel());
            //if(config->p25_logging > 2) printf("   mx:%f, mn:%f, ct:%f, std:%f", max, min, center, mean_deviation);
          }
          else if(!debug && config->p25_logging>1) {
            printf("\r\n  ->LDU1 (VOICE), freq: %3.4f, rssi: %d, ch: %d",  get_current_freq(), get_rssi(), get_current_channel());
            //if(config->p25_logging > 2) printf("   mx:%f, mn:%f, ct:%f, std:%f", max, min, center, mean_deviation);
          }

					if(!found_header) {
						found_header=1;
						//if(out_fd!=0) close(out_fd);
						//out_fd=0;
					}
					if(working_state==1) working_state++;
						else working_state=0;
				break;

				case	0x0a	:	
					is_synced=1200;
					if(prev_duid!=0x05)  {
						voice_state=-1;
						bit_count=0;
						found_header=0;
						//if(out_fd!=0) close(out_fd);
						//out_fd=0;
						//flush_audio();
						return 0;
					}
					else {
						if(working_state==2) working_state++;
						else if(working_state==1) working_state+=2;
							else working_state=0;
					}


					if(talkgroup!=0 && !debug && config->p25_logging>1) printf("\r\n  ->LDU2 (VOICE) talkgroup: %u", talkgroup);
						else if(!debug && config->p25_logging>1) printf("\r\n  ->LDU2 (VOICE)");

						if(talkgroup!=0) eq_state++;
				break;

				case	0x03	:	
					if(!debug && config->p25_logging>1) printf("\r\n  ->TDU");
					is_synced=1200;
          set_channel_timeout_tdulc();

          //set_channel_timeout_min();

				//	duid=-1;
				//		if(working_state==3) working_state++;
				//			else working_state=0;
				break;

				case	0x0f	:	

					if(!debug && config->p25_logging>1) printf("\r\n  ->TDULC");
					//if(out_fd!=0) close(out_fd);
					//out_fd=0;
					//is_synced=0;
          set_channel_timeout_tdulc();
					found_header=0;
					duid=-1;
					eq_state=0;

						is_playing=1;	

						//save working levels since we got this far
						if(working_state==4) { 
							working_state=0;
							working_center = center;
							working_max = max;
							working_min = min;
						}
					//flush_audio();

				break;

				default	:
					//if(out_fd!=0) close(out_fd);
					//out_fd=0;
					//is_synced=0;
					found_header=0;
					duid=-1;
					working_state=0;
					voice_state=-1;
				break;
			}

			prev_duid = duid;
		}
	}
		//found_header=0;
		if(!dibit_skip && is_synced && duid==0x00 && voice_state==0 && bit_count>66+(9*12*2) ) {
			if(word_count<8*9) {
				hdu_hex_data[word_count++] = dreg&0x03;
				if(word_count==8*9) {
					p25_processHDU( hdu_hex_data );
				}
			}
		}

		//voice unit
		if(!dibit_skip && is_synced && ( duid==0x05 || duid==0x0a) ) {

			switch( voice_state ) {
				case	0	:
					if( bit_count == 66) {	//NID bits
						bit_count = 0;
						voice_state=1;
						status_bit_counter = 22;
					}
				break;

				case	1 :	//VOICE1

					deinterleave_frame(bit_count-2, (uint8_t)(dreg&0x03));
					if(bit_count==144) {

						//if(algid!=0x00) break;

						memset(imbe_d,0x00,88);

            //if(talkgroup==0) do_mute=1;
            //if( algid==0x80 ) do_mute=1; 
            if( algid==0x80 ) {
              do_mute=1; 
            }
            else {
              do_mute=0;
              algid=0x00; //don't mute
            }

						//if(!found_header || algid==0x80 || (algid!=0x00 && mfid!=0x08) || do_mute ) {
						//	found_header=0;
						//	voice_state=-1;
						//	//is_synced=0;  
						//	return 0;
						//}

						errors=p25_mbe_eccImbe7200x4400C0(imbe_fr);
						errors2 = errors;
						p25_mbe_demodulateImbe7200x4400Data(imbe_fr);
						errors2 += p25_mbe_eccImbe7200x4400Data(imbe_fr, imbe_d);
						if(found_header && !do_mute) process_mbe(imbe_d, errors, errors2, algid, 0);

						//else if(out_fd==0) { 
					//		sprintf(filename, "/tmp/p25_out%06d.imb\0", file_n++);
					//		out_fd = open(filename, O_WRONLY | O_CREAT, 0644);
					//		if(out_fd!=0) write(out_fd, header,4);
					//	}
						//pack_data(imbe_d, voice_data, errors);
						//if(found_header && out_fd!=0) write(out_fd, voice_data,12);
						bit_count=0;
						voice_state=2;
					}
				break;

				case	2 :	//VOICE2
					deinterleave_frame(bit_count-2, (uint8_t)(dreg&0x03));
					if(bit_count==144) {
						//if(algid!=0x00) break;
						memset(imbe_d,0x00,88);

						errors=p25_mbe_eccImbe7200x4400C0(imbe_fr);
						errors2 = errors;
						p25_mbe_demodulateImbe7200x4400Data(imbe_fr);
						errors2 += p25_mbe_eccImbe7200x4400Data(imbe_fr, imbe_d);
						if(found_header && !do_mute) process_mbe(imbe_d, errors, errors2, algid, 0);

						//pack_data(imbe_d, voice_data, errors);
						//if(found_header && out_fd!=0) write(out_fd, voice_data,12);
						bit_count=0;
						voice_state=3;
						hex_bit_count=0;	//only reset here in the VOICE frames to collect total of 40 * 6 bits
					}
				break;

				case	3 :	//4 blocks of 6-bits data + 4-bits parity for 40 bits total
					if(bit_count<40) {
						ldu_hex_parity[hex_bit_count++] = (dreg&0x03);
					}
					else {
						bit_count=0;
						voice_state=4;
					}
				break;

				case	4 :	//VOICE3
					deinterleave_frame(bit_count-2, (uint8_t)(dreg&0x03));
					if(bit_count==144) {
						//if(algid!=0x00) break;
						memset(imbe_d,0x00,88);

						errors=p25_mbe_eccImbe7200x4400C0(imbe_fr);
						errors2 = errors;
						p25_mbe_demodulateImbe7200x4400Data(imbe_fr);
						errors2 += p25_mbe_eccImbe7200x4400Data(imbe_fr, imbe_d);
						if(found_header && !do_mute) process_mbe(imbe_d, errors, errors2, algid, 0);

						//pack_data(imbe_d, voice_data, errors);
						//if(found_header && out_fd!=0) write(out_fd, voice_data,12);
						bit_count=0;
						voice_state=5;
					}
				break;

				case	5 :	//4 blocks of 6-bits data + 4-bits parity for 40 bits total
					if(bit_count<40) {
						ldu_hex_parity[hex_bit_count++] = (dreg&0x03);
					}
					else {
						bit_count=0;
						voice_state=6;
					}
				break;

				case	6 :	//VOICE4
					deinterleave_frame(bit_count-2, (uint8_t)(dreg&0x03));
					if(bit_count==144) {
						//if(algid!=0x00) break;
						memset(imbe_d,0x00,88);

						errors=p25_mbe_eccImbe7200x4400C0(imbe_fr);
						errors2 = errors;
						p25_mbe_demodulateImbe7200x4400Data(imbe_fr);
						errors2 += p25_mbe_eccImbe7200x4400Data(imbe_fr, imbe_d);
						if(found_header && !do_mute) process_mbe(imbe_d, errors, errors2, algid, 0);

						//pack_data(imbe_d, voice_data, errors);
						//if(found_header && out_fd!=0) write(out_fd, voice_data,12);
						bit_count=0;
						voice_state=7;
					}
				break;

				case	7 :	//4 blocks of 6-bits data + 4-bits parity for 40 bits total
					if(bit_count<40) {
						ldu_hex_parity[hex_bit_count++] = (dreg&0x03);
					}
					else {
						bit_count=0;
						voice_state=8;
					}
				break;

				case	8 :	//VOICE5
					deinterleave_frame(bit_count-2, (uint8_t)(dreg&0x03));
					if(bit_count==144) {
						//if(algid!=0x00) break;
						memset(imbe_d,0x00,88);

						errors=p25_mbe_eccImbe7200x4400C0(imbe_fr);
						errors2 = errors;
						p25_mbe_demodulateImbe7200x4400Data(imbe_fr);
						errors2 += p25_mbe_eccImbe7200x4400Data(imbe_fr, imbe_d);
						if(found_header && !do_mute) process_mbe(imbe_d, errors, errors2, algid, 0);

						//pack_data(imbe_d, voice_data, errors);
						//if(found_header && out_fd!=0) write(out_fd, voice_data,12);
						bit_count=0;
						voice_state=9;
					}
				break;

				case	9 :	//4 blocks of 6-bits data + 4-bits parity for 40 bits total
					if(bit_count<40) {
						ldu_hex_parity[hex_bit_count++] = (dreg&0x03);
					}
					else {
						bit_count=0;
						voice_state=10;
					}
				break;

				case	10 :	//VOICE6
					deinterleave_frame(bit_count-2, (uint8_t)(dreg&0x03));
					if(bit_count==144) {
						//if(algid!=0x00) break;
						memset(imbe_d,0x00,88);

						errors=p25_mbe_eccImbe7200x4400C0(imbe_fr);
						errors2 = errors;
						p25_mbe_demodulateImbe7200x4400Data(imbe_fr);
						errors2 += p25_mbe_eccImbe7200x4400Data(imbe_fr, imbe_d);
						if(found_header && !do_mute) process_mbe(imbe_d, errors, errors2, algid, 0);

						//pack_data(imbe_d, voice_data, errors);
						//if(found_header && out_fd!=0) write(out_fd, voice_data,12);
						bit_count=0;
						voice_state=11;
					}
				break;

				case	11 :	//4 blocks of 6-bits data + 4-bits parity for 40 bits total
					if(bit_count<40) {
						ldu_hex_parity[hex_bit_count++] = (dreg&0x03);
					}
					else {
						bit_count=0;
						voice_state=12;
					}
				break;

				case	12 :	//VOICE7
					deinterleave_frame(bit_count-2, (uint8_t)(dreg&0x03));
					if(bit_count==144) {
						//if(algid!=0x00) break;
						memset(imbe_d,0x00,88);

						errors=p25_mbe_eccImbe7200x4400C0(imbe_fr);
						errors2 = errors;
						p25_mbe_demodulateImbe7200x4400Data(imbe_fr);
						errors2 += p25_mbe_eccImbe7200x4400Data(imbe_fr, imbe_d);
						if(found_header && !do_mute) process_mbe(imbe_d, errors, errors2, algid, 0);

						//pack_data(imbe_d, voice_data, errors);
						//if(found_header && out_fd!=0) write(out_fd, voice_data,12);
						bit_count=0;
						voice_state=13;
					}
				break;

				case	13 :	//4 blocks of 6-bits data + 4-bits parity for 40 bits total
					if(bit_count<40) {
						ldu_hex_parity[hex_bit_count++] = (dreg&0x03);
					}
					else {
						bit_count=0;
						voice_state=14;
					}
				break;

				case	14 :	//VOICE8
					deinterleave_frame(bit_count-2, (uint8_t)(dreg&0x03));
					if(bit_count==144) {
						//if(algid!=0x00) break;
						memset(imbe_d,0x00,88);

						errors=p25_mbe_eccImbe7200x4400C0(imbe_fr);
						errors2 = errors;
						p25_mbe_demodulateImbe7200x4400Data(imbe_fr);
						errors2 += p25_mbe_eccImbe7200x4400Data(imbe_fr, imbe_d);
						if(found_header && !do_mute) process_mbe(imbe_d, errors, errors2, algid, 0);

						//pack_data(imbe_d, voice_data, errors);
						//if(found_header && out_fd!=0) write(out_fd, voice_data,12);
						bit_count=0;
						voice_state=15;
					}
				break;

				case	15 :	//LSD=32
					if(bit_count<32) {
					}
					else {
						bit_count=0;
						voice_state=16;
					}
				break;

				case	16 :	//VOICE9
					deinterleave_frame(bit_count-2, (uint8_t)(dreg&0x03));
					if(bit_count==144) {
						//if(algid!=0x00) break;
						memset(imbe_d,0x00,88);

						errors=p25_mbe_eccImbe7200x4400C0(imbe_fr);
						errors2 = errors;
						p25_mbe_demodulateImbe7200x4400Data(imbe_fr);
						errors2 += p25_mbe_eccImbe7200x4400Data(imbe_fr, imbe_d);
						if(found_header && !do_mute) process_mbe(imbe_d, errors, errors2, algid, 1);

						//pack_data(imbe_d, voice_data, errors);

						//only process for LDU1 type	
						//if(duid==0x05) process_LDU_Header( (char *) ldu_hex_parity );	//doesn't work right.  ok since HDU frame works for everything needed

						//if(found_header && out_fd!=0) write(out_fd, voice_data,12);

						bit_count=0;
						voice_state=0;
						duid=-1;

					}
				break;

			}

		}


		return cval;
}



///////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////
void deinterleave_frame(int dibit_n, uint8_t dibit_val) {


	if(dibit_n==0) stime = HAL_GetTick();

	if(dibit_n>142) dibit_n = 0;

	int i = dibit_n/2;

	imbe_fr[iW[i]][iX[i]] = (1 & (dibit_val >> 1)); // bit 1
	imbe_fr[iY[i]][iZ[i]] = (1 & dibit_val);        // bit 0
}


///////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////
int p25_mbe_eccImbe7200x4400C0 (char imbe_fr[8][23]) {

  int j, errs;
  char in[23], out[23];

  for (j = 0; j < 23; j++)
    {
      in[j] = imbe_fr[0][j];
    }
  errs = mbe_golay2312 (in, out);
  for (j = 0; j < 23; j++)
    {
      imbe_fr[0][j] = out[j];
    }

  return (errs);

}

///////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////
void p25_mbe_demodulateImbe7200x4400Data (char imbe[8][23]) {
  int i, j = 0, k;
  unsigned short pr[115], foo = 0;

  // create pseudo-random modulator
  for (i = 11; i >= 0; i--) {
      foo <<= 1;
      foo |= imbe[0][11+i];
  }

  pr[0] = (16 * foo);
  for (i = 1; i < 115; i++) {
      pr[i] = (173 * pr[i - 1]) + 13849 - (65536 * (((173 * pr[i - 1]) + 13849) >> 16));
  }
  for (i = 1; i < 115; i++) {
      pr[i] >>= 15;
  }

  // demodulate imbe with pr
  k = 1;
  for (i = 1; i < 4; i++) {
      for (j = 22; j >= 0; j--)
        imbe[i][j] ^= pr[k++];
  }
  for (i = 4; i < 7; i++) {
      for (j = 14; j >= 0; j--)
        imbe[i][j] ^= pr[k++];
  }
}

///////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////
int p25_mbe_eccImbe7200x4400Data (char imbe_fr[8][23], char *imbe_d) {
  int i, j, errs;
  char *imbe, gin[23], gout[23], hin[15], hout[15];

  errs = 0;
  imbe = imbe_d;
  for (i = 0; i < 4; i++)
    {
      if (i > 0)
        {
          for (j = 0; j < 23; j++)
            {
              gin[j] = imbe_fr[i][j];
            }
          errs += mbe_golay2312 (gin, gout);
          for (j = 22; j > 10; j--)
            {
              *imbe = gout[j];
              imbe++;
            }
        }
      else
        {
          for (j = 22; j > 10; j--)
            {
              *imbe = imbe_fr[i][j];
              imbe++;
            }
        }
    }
  for (i = 4; i < 7; i++)
    {
      for (j = 0; j < 15; j++)
        {
          hin[j] = imbe_fr[i][j];
        }
      errs += mbe_hamming1511 (hin, hout);
      for (j = 14; j >= 4; j--)
        {
          *imbe = hout[j];
          imbe++;
        }
    }
  for (j = 6; j >= 0; j--)
    {
      *imbe = imbe_fr[7][j];
      imbe++;
    }

  return (errs);
}


///////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////
void pack_data(char *in, char *out, int errors) {
  int i, j, k;
  unsigned char b=0;

	out[0] = (uint8_t) errors; 
	//out[0] = 0; //no errors 

  k = 0;
  for (i = 0; i < 11; i++) {

    for (j = 0; j < 8; j++) {
			b<<=1;
			if( in[k++] ) b|=1;
    }

		out[i+1] = b;
  }
}


//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////
void p25_processHDU( char *hex_and_parity) {

unsigned char hex_data[8];    // Data in hex-words (6 bit words). A total of 8 hex words.
int i;

	errors = 0;
	for (i = 0; i < 8; i++) {
		// read both the hex word and the Golay(23, 12) parity information
		//read_dibit(opts, state, hex_and_parity, 9, &status_count);
		// Use the Golay23 FEC to correct it.
		hex_data[i] = hdu_correct_hex_word((unsigned char *) &hex_and_parity[i*9], 9);
		//if(gerr>3) errors++; //uncorrectable.  should be 3 errors?
		if(gerr>2) errors++; //uncorrectable
	}
		


	if(errors==0) {


		mfid = hex_data[0];
		mfid <<=8;
		mfid |= hex_data[1];

		//algid  = (((hex_data[1] >> 2) << 4) | (hex_data[2] & 0x0f));
		algid = (hex_data[2]<<3)&0xff;	

		talkgroup  = (((hex_data[5] >> 2) << 12) | (hex_data[6] << 6) | (hex_data[7]));

		//validate header as best we can
		if( (mfid==config->p25_radiomanf_id || config->p25_radiomanf_id_allow_all) && (algid==0x00 || algid==0x80) && (nac==config->p25_network_id || config->p25_network_id_allow_all) ) { 

			if(!debug && config->p25_logging>0) printf("\r\n  NAC %d, MFID %d, TG %u, Freq: %3.4f", (unsigned int) nac, mfid, talkgroup, get_current_freq());
			if( algid == 0x80 && !debug && config->p25_logging>0) printf("  (ENCRYPTED) ->mute");
			//for (i = 0; i < 8; i++) {
			//	printf("  %02x,",hex_data[i]);
			//}
			if(config->p25_logging>0) printf("\r\n  %s\r\n", get_talkgroup_str(talkgroup));
		}
		else {
			talkgroup=0x00;  //zero this out since we aren't sure it is right
		}
	}
	else {
		if(config->p25_logging>1) printf("\r\nHDU contains errors");
	}

}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
unsigned int hdu_correct_hex_word(unsigned char *hex_and_parity, unsigned int codeword_bits) {
  unsigned int i, golay_codeword = 0;


  // codeword now contains:
  // bits 0-10: golay (23,12) parity bits
  // bits 11-22: hex bits
  for (i = 0; i < codeword_bits; i++) {
    golay_codeword <<= 2;
    golay_codeword |= hex_and_parity[i];
  }
  golay_codeword >>= 1;

  gerr = Golay23_CorrectAndGetErrCount(&golay_codeword);

	//if(gerr>3) if(config->p25_logging>1) printf("\r\nhdu error: %d", gerr);

  return golay_codeword;
}


///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////
static unsigned int GolayGenerator[12] = {
  0x63a, 0x31d, 0x7b4, 0x3da, 0x1ed, 0x6cc, 0x366, 0x1b3, 0x6e3, 0x54b, 0x49f, 0x475
};

///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////
void Golay23_Correct(unsigned int *block) {
  unsigned int i, syndrome = 0;
  unsigned int mask, block_l = *block;

  mask = 0x400000l;
  for (i = 0; i < 12; i++) {
      if ((block_l & mask) != 0) {
          syndrome ^= GolayGenerator[i];
      }
      mask = mask >> 1;
  }

  syndrome ^= (block_l & 0x07ff);
  *block = ((block_l >> 11) ^ GolayMatrix[syndrome]);
}


///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////
char *get_talkgroup_str(int talkgroup) {

	memset(talkgroup_str,0x00,sizeof(talkgroup_str));

	switch(talkgroup) {

		case	300	:
			strcpy(talkgroup_str, "Law All Call  Law All Call\0");
		break;

		case	301 :
			strcpy(talkgroup_str,  "Pasco PD Dispatch\0");
			break;

		case	305 :
			strcpy(talkgroup_str,  "LERN  Statewide Law Net\0");
			break;

		case	811 :
			strcpy(talkgroup_str,  "WSP Kennewick Dispatch\0");
			break;

		case	203 :
			strcpy(talkgroup_str,  "BC DEM OPS  Emergency Mgmt Operations\0");
			break;

		case	207 :
			strcpy(talkgroup_str,  "BC DEM CMD  Emergency Mgmt Command\0");
			break;

		case	213 :
			strcpy(talkgroup_str,  "BC DEM DEPOT  Emergency Mgmt Depot\0");
			break;

		case	217 :
			strcpy(talkgroup_str,  "BC DEM EVAC   Emergency Mgmt Evacuation\0");
			break;

		case	307 :
			strcpy(talkgroup_str,  "BCSO Tac 6  Sheriff Tac 6\0");
			break;

		case	309 :
			strcpy(talkgroup_str,  "BCSO Data   Sheriff Data\0");
			break;

		case	311 :
			strcpy(talkgroup_str,  "Benton Law  Regional Law Net\0");
			break;

		case	313 :
			strcpy(talkgroup_str,  "PD Emergency  PD Emergency\0");
			break;

		case	317 :
			strcpy(talkgroup_str,  "BCSO Disp   Sheriff Dispatch\0");
			break;

		case	347 :
			strcpy(talkgroup_str,  "BCSO 3 C2C  Sheriff 3 C2C\0");
			break;

		case	353 :
			strcpy(talkgroup_str,  "BCSO Tac 16   Sheriff Tac 16\0");
			break;

		case	525 :
			strcpy(talkgroup_str,  "BCFD Dist 6   Fire District 6\0");
			break;

		case	541 :
			strcpy(talkgroup_str,  "BCFD D6 Admn  Fire District 6\0");
			break;

		case	581 :
			strcpy(talkgroup_str,  "Metro 9 SOPS  Special Operations\0");
			break;
		
		case	833 :
			strcpy(talkgroup_str,  "BCSO Jail   Jail Operations\0");
			break;

		case	835 :
			strcpy(talkgroup_str,  "BCSO Jail 2   Jail Operations 2\0");
			break;

		case	837 :
			strcpy(talkgroup_str,  "BCSO Jail Ma  Jail Maintenance\0");
			break;

		case	853 :
			strcpy(talkgroup_str,  "BCPW Road Op  Road Dept Operations\0");
			break;

		case	855 :
			strcpy(talkgroup_str,  "BCPW Road Ke  Road Dept Kennewick\0");
			break;

		case	857 :
			strcpy(talkgroup_str,  "BCPW Road Pr  Road Dept Prosser\0");
			break;

		case	859 :
			strcpy(talkgroup_str,  "BCPW Road Su  Road Dept Survey\0");
			break;

		case	861 :
			strcpy(talkgroup_str,  "BCPW Road PE  Road Dept Project Engineers\0");
			break;

		case	863 :
			strcpy(talkgroup_str,  "BCSO Bailiff  Jail Bailiffs\0");
			break;

		case	865 :
			strcpy(talkgroup_str,  "BCSO Jail MC  Jail Master Control\0");
			break;

		case	867 :
			strcpy(talkgroup_str,  "BCSO Warrant  Sheriff Warrant Service\0");
			break;

		case	869 :
			strcpy(talkgroup_str,  "BCSO JJC  Juvenile Center Ops\0");
			break;

		case	872 :
			strcpy(talkgroup_str,  "BCSO Jail\0");
			break;

		case	303 :
			strcpy(talkgroup_str,  "KPD Dispatch  Police Dispatch\0");
			break;

		case	343 :
			strcpy(talkgroup_str,  "KPD Car   Police Car to Car\0");
			break;

		case	319 :
			strcpy(talkgroup_str,  "PPD Dispatch  Police Dispatch\0");
			break;

		case	329 :
			strcpy(talkgroup_str,  "PPD Tac 2   Police Tactical 2\0");
			break;

		case	333 :
			strcpy(talkgroup_str,  "PPD Car   Police Car to Car\0");
			break;

		case	339 :
			strcpy(talkgroup_str,  "PPD Car   Police Car to Car\0");
			break;

		case	591 :
			strcpy(talkgroup_str,  "Prosser Amb   Prosser Ambulance\0");
			break;

		case	345 :
			strcpy(talkgroup_str,  "RPD Car   Police Car to Car\0");
			break;

		case	351 :
			strcpy(talkgroup_str,  "Pasco Car to Car / Undercover?\0");	//animal control duty??
			break;

		case	349 :
			strcpy(talkgroup_str,  "RPD 3 West  Police 3 West C2C\0");
			break;

		case	873 :
			strcpy(talkgroup_str,  "Areva Sec   Areva Security\0");
			break;

		case	111 :
		case	885 :
			strcpy(talkgroup_str,  "City Workers/Security Firms/?\0");
			break;

		default	:
			strcpy(talkgroup_str,  "Uknown\0");
		break;

	}
		return talkgroup_str;
}
