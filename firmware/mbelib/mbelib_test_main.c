#include "dsd.h"
#include "mbelib_test_main.h"

int imb_idx;
dsd_state dsdstate;
dsd_state *t_state;
dsd_opts dsdopts;
dsd_opts *t_opts;
char t_imbe_d[88];
float dsd_fbuf[1024];
short dsd_sbuf[1024];
int as_count;
int dsd_sample_count;

static mbe_parms mbe1;
static mbe_parms mbe2;
static mbe_parms mbe3;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void mbe_test_tick()
{
  //t_opts->uvquality = 3;
  t_opts->uvquality = 1;

  readImbe4400Data(t_opts, t_state, t_imbe_d);


  //mbe_processImbe4400Dataf (dsd_fbuf, &t_state->errs, &t_state->errs2, t_state->err_str, t_imbe_d, t_state->cur_mp, t_state->prev_mp, t_state->prev_mp_enhanced, t_opts->uvquality);

  mbe_decodeImbe4400Parms(t_imbe_d, t_state->cur_mp, t_state->prev_mp);
  mbe_moveMbeParms (t_state->cur_mp, t_state->prev_mp);
  mbe_spectralAmpEnhance (t_state->cur_mp);
  mbe_synthesizeSpeechf (dsd_fbuf, t_state->cur_mp, t_state->prev_mp_enhanced, t_opts->uvquality);


  mbe_processAudio(dsd_fbuf, dsd_sbuf);  //convert float to shorts ... 160 of them.  Output is 8 kHz
  //audio in dsd_sbuf is 8000 Hz
  playSynthesizedVoice(t_opts, t_state);
  dsd_sample_count+=160;

  //printf("\r\nimb_idx %d, audio len: %d", imb_idx, as_count);
  //printf("\r\n");
  //for(i=0;i<160;i++) {
  // printf("%d,", (short) dsd_sbuf[i]);
  //}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void mbelib_test_init(void) {

  t_state = &dsdstate;
  t_opts = &dsdopts;

  t_opts->onesymbol = 10;
  t_opts->mbe_in_file[0] = 0;
  t_opts->mbe_in_f = NULL;
  t_opts->errorbars = 1;
  t_opts->datascope = 0;
  t_opts->symboltiming = 0;
  t_opts->verbose = 2;
  t_opts->p25enc = 0;
  t_opts->p25lc = 0;
  t_opts->p25status = 0;
  t_opts->p25tg = 0;
  t_opts->scoperate = 15;
  t_opts->audio_in_fd = -1;
  t_opts->split = 0;
  t_opts->playoffset = 0;
  t_opts->mbe_out_dir[0] = 0;
  t_opts->mbe_out_file[0] = 0;
  t_opts->mbe_out_path[0] = 0;
  t_opts->mbe_out_f = NULL;
  t_opts->audio_gain = 0;
  t_opts->audio_out = 1;
  t_opts->wav_out_file[0] = 0;
  t_opts->wav_out_f = NULL;
  t_opts->serial_baud = 115200;
  t_opts->resume = 0;
  t_opts->frame_dstar = 0;
  t_opts->frame_x2tdma = 1;
  t_opts->frame_p25p1 = 1;
  t_opts->frame_nxdn48 = 0;
  t_opts->frame_nxdn96 = 1;
  t_opts->frame_dmr = 1;
  t_opts->frame_provoice = 0;
  t_opts->mod_c4fm = 1;
  t_opts->mod_qpsk = 1;
  t_opts->mod_gfsk = 1;
  t_opts->uvquality = 3;
  t_opts->inverted_x2tdma = 1;    // most transmitter + scanner + sound card combinations show inverted signals for this
  t_opts->inverted_dmr = 0;       // most transmitter + scanner + sound card combinations show non-inverted signals for this
  t_opts->mod_threshold = 26;
  t_opts->ssize = 36;
  t_opts->msize = 15;
  t_opts->playfiles = 0;
  t_opts->delay = 0;
  t_opts->use_cosine_filter = 1;
  t_opts->unmute_encrypted_p25 = 0;



	t_state->cur_mp = &mbe1; 
	t_state->prev_mp = &mbe2; 
	t_state->prev_mp_enhanced = &mbe3; 


  t_state->mbe_file_type = 0;  //imb ==0, ambe=1
  imb_idx=4;
  mbe_initMbeParms (t_state->cur_mp, t_state->prev_mp, t_state->prev_mp_enhanced);


}


///////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////
void playSynthesizedVoice (dsd_opts * opts, dsd_state * state)
{
  //state->audio_out_buf_p    //start address of buffer
  //state->audio_out_idx      //length of buffer
}

///////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////
unsigned char imb_fgetc(dsd_opts *opts)
{
	/*
  if(imb_idx < 6232) {
    return (unsigned char) test_imb[imb_idx++];
  } else {
    printf("\r\nstop");
    printf("\r\nstart %d, sample_count: %d", as_count++, dsd_sample_count);
    imb_idx = 4;
    return (unsigned char) test_imb[imb_idx++];
  }
	*/
		return 0;
}

///////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////
int readImbe4400Data (dsd_opts * opts, dsd_state * state, char *imbe_d)
{
  int i, j, k;
  unsigned char b;

  state->errs2 = imb_fgetc(opts);
  state->errs = state->errs2;

  k = 0;

  for (i = 0; i < 11; i++) {

    b = imb_fgetc (opts);

    for (j = 0; j < 8; j++) {
      imbe_d[k] = (b & 128) >> 7;
      b = b << 1;
      b = b & 255;
      k++;
    }
  }
  return (0);
}

///////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////
int mbe_processAudio(float *in_f, short *out_s)
{

  int i;

  for(i=0; i<160; i++) {
    //*out_s++ = (short) ((float) *in_f++ * 2.0f);
    *out_s++ = (short) ((float) *in_f++ );
  }

  return 160;
}
