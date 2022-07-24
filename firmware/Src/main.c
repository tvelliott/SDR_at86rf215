
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




/* Includes ------------------------------------------------------------------*/
#include <complex.h>
#include <math.h>

#include "globals.h"
#include "main.h"
#include "stm32h7xx_hal.h"
#include "lwip.h"
#include "mbelib_test_main.h"

#define ARM_MATH_CM7 1
#include "arm_common_tables.h"
#include "arm_math.h"

#include "symsync_config.h"
#include "firdes_arm32.h"
#include "symsync_rf_arm32.h"
#include "freqdem_cfrf_arm32.h"
#include "resamp_cf_arm32.h"
#include "resamp_rf_arm32.h"
#include "window_functions_arm32.h"

#include "math_arm32.h"
#include "msb_index_arm32.h"
#include "firpfb_cf_arm32.h"
#include "window_cf_arm32.h"
#include "dotprod_cf_arm32.h"
#include "resamp_cf_arm32.h"
#include "resamp2_cf_arm32.h"
#include "rkaiser_arm32.h"
#include "window_functions_arm32.h"
#include "firdes_arm32.h"
#include "resamp_config.h"
#include "symsync_config.h"
#include "firfilt_rf_arm32.h"
#include "firfilt_cf_arm32.h"
#include "nco_cf_arm32.h"
#include "freqmod_arm32.h"
#include "eqlms_rf_arm32.h"
#include "wdelay_rf_arm32.h"


#include "p25_decode.h"
#include "p25_stats.h"
#include "channel_filter.h"

#include "telnet.h"

#include "lwip/udp.h"

#include "flash_s08.h"
#include "fpga.h"
#include "at86rf215.h"

#include "main.h"

/* USER CODE BEGIN Includes */
//#include "liquid.h"

/* USER CODE END Includes */
// create resampler
void eval_fir( float *data_in, float *data_out, int len, float *zbuffer, float *coef );

#define FLT_LEN 33

float ii_off;
float qq_off;

float zbuffer_i[97];
float zbuffer_q[97];
float data_in_i[256];
float data_out_i[256];
float data_in_q[256];
float data_out_q[256];

//remove dc level
float ALPHA_D_i;
float d_alpha_i;
float d_beta_i;
float d_avg_i;

float ALPHA_D_q;
float d_alpha_q;
float d_beta_q;
float d_avg_q;



static int current_rssi = -127;

//filter len=97
float sinc_filter[] = {
  0.140496194363, 0.281820982695, -0.090696640313, -0.046851862222, -0.164575159550, -0.134356126189, -0.031023621559, 0.179854497313, 0.375539124012, 0.026221035048
  , -0.028105402365, -0.326236367226, -0.382247179747, -0.202509149909, 0.184361815453, 0.638917803764, 0.576637566090, 0.638917803764, 0.184361815453, -0.202509149909
  , -0.382247179747, -0.326236367226, -0.028105402365, 0.026221035048, 0.375539124012, 0.179854497313, -0.031023621559, -0.134356126189, -0.164575159550, -0.046851862222
  , -0.090696640313, 0.281820982695, 0.140496194363
};

/* Private variables ---------------------------------------------------------*/
static struct pbuf *p;
volatile struct ip4_addr udp_saddr;
volatile static char udp_send_addr[4] = {0xc0, 0xa8, 0x01, 0x03};

volatile int agc_locked;

#define N_SAMPLES 256

float audio_out_level = 10000.0f;

volatile int do_freq_offset = 1;

volatile int do_48khz_iq_only = 0; //1==do RF IQ resamp from 100khz to 48khz only and send that over udp

volatile int do_low_if_mix = 0; //1==do the entire dsp chain and send 48khz audio over udp with low-if 12.5khz offset
//(much better receiver performance with low-if offset)
//0==do the entire dsp chain and send 48khz audio over udp,  DONT do low-if 12.5khz offset

float32_t stddev_in[N_SAMPLES];
float32_t stddev_out;
int stddev_n;

volatile int channel_timeout;
volatile int scanner_channel = 2;
volatile uint32_t scanner_idle_time;
volatile int8_t std_dev;
//1 (1)   001 (1)   Simulcast 854.4125  854.5125  856.9375  856.9625  857.9375  857.9625  858.9375a   858.9625a

volatile int scanner_freqs = 4;
volatile int scanner_channel_mask[] = { 1, 1, 1, 1 }; //skip last channel (too weak)
//volatile float scanner_freq_p25[] = { 856.9375, 856.9625, 857.9375, 857.9625 };
volatile float scanner_freq_p25[] = { 954.5 + 1.025, 954.5 + 1, 954.5 + 0.025, 954.5 };
volatile int scanner_gain[] = {8, 12, 8, 12}; //8=good to -58dBm,  12=good to -64dBm
volatile int scanner_thresh[] = {-74, -83, -74, -83 };

volatile int did_print_channel;
static volatile float real_amp = 0.0f;

UART_HandleTypeDef huart3;
ADC_HandleTypeDef hadc1;
ADC_HandleTypeDef hadc3;
SD_HandleTypeDef hsd2;
SPI_HandleTypeDef hspi3;

void p25_net_tick( void );
void process_incoming_samples( int16_t val );
void udp_send_data( uint8_t *buffer, int len );
void p25_udp_init( void );
void udp_data_rx( void *arg, struct udp_pcb *pcb, struct pbuf *p, struct ip4_addr *addr, uint16_t port );
int _mem_free( void );
void led_tick( void );
void main_tick( void );

uint32_t second_tick;
volatile uint32_t uptime_sec;
int sample_cnt;
int found_signal;

#define IN_BUFFER_N 16

char in_buffer[1500 * IN_BUFFER_N];
char in_buffer_tmp[1500 * IN_BUFFER_N];
int in_buffer_len;

volatile int scanner_mod;

/* USER CODE BEGIN PV */
/* Private variables ---------------------------------------------------------*/
int64_t __errno;

static int set_mco2 = 0;
static float complex fm_in[8];
static float fm_out[1024];

volatile static int avg_i;
volatile static int avg_q;
volatile static int avg_iq_cnt;

volatile struct udp_pcb *udp_data_pcb;
int audio_frames_done;
int do_silent_frame;

int eqlms_trained = 0;

volatile int fm_cnt;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config( void );
static void MX_GPIO_Init( void );
static void MX_USART3_UART_Init( void );

static void MX_ADC1_Init( void );
static void MX_ADC3_Init( void );
static void MX_SDMMC2_SD_Init( void );
void MX_SPI3_Init( void );



/* USER CODE BEGIN PFP */
/* Private function prototypes -----------------------------------------------*/
static void MPU_Config( void );
void tcp_iperf_init( void );
static volatile uint32_t current_time;
static uint32_t led_time;
static uint8_t led_state;
static uint32_t clk_mhz;

// initialize frame generator properties
//flexframegenprops_s fgprops;

/* USER CODE END PFP */

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

unsigned int did_sym = 0;

// options
int          symsync_ftype = LIQUID_FIRFILT_RRC; // filter type
unsigned int symsync_k           =   10;     // filter samples/symbol  48Ksps/4.8k symbols/sec
unsigned int symsync_m           =   2;     // number of bits/symbol  4-FSK
float        symsync_beta        = 0.365f;    //filter excess bandwidth factor (best somewhere between 0.36 to 0.37)
unsigned int symsync_npfb        = SYMSYNC_NFILTERS;  //number of polyphase filters in bank

float symsync_buf_in[1];
float symsync_buf_out[1];


struct symsync_s symsync;
struct symsync_s *q;

static uint32_t telnet_tick_time;

#define SAMPLE_FREQ (100.0e3*2.0)     //input RF sample rate
#define RF_CHANNEL_BW  12500.0f   //bw fed into FM demodulator (FM deviation)
#define POST_FM_FLT_BW  12500.0f  //bw after FM demodulator  (audio)
#define DECODER_FS  (47937.00f/1.0)     //final re-sampled rate of audio

int decimate;

//I/Q from transceiver will be 400 Ksps,  FPGA decimated to 100 Ksps
//down-sample incoming 100 Ksps I/Q from RF channel to desired rate of 48Ksps
//float ch_resamp_r           = 2.0f*(DECODER_FS/SAMPLE_FREQ); // resampling rate (output/input)
float ch_resamp_r           = 3.84 / 2.0; // resampling rate (output/input)

unsigned int ch_resamp_m    = RESAMP_CF_M;     // resampling filter semi-length (filter delay)
float ch_resamp_As          = -60.0f;  // resampling filter stop-band attenuation [dB]
//float ch_resamp_bw          = (RF_CHANNEL_BW/SAMPLE_FREQ);  // resampling filter bandwidth
//float ch_resamp_bw          = (RF_CHANNEL_BW/SAMPLE_FREQ)/2.0f;  // resampling filter bandwidth
float ch_resamp_bw          = 0.55;  // resampling filter bandwidth
unsigned int ch_resamp_npfb = RESAMP_CF_NFILTERS;     // number of filters in bank (timing resolution)
static float complex samples[256];

static struct resamp_cf_s ch_rs;
struct resamp_cf_s *ch_resamp_q;

unsigned int half_band_m = 7;
float half_band_as = -60.0f;
float complex half_band_x;
float complex half_band_2x[2];

static struct resamp2_s halfband_s;
struct resamp2_s *halfband_q;

float complex hb_samples1[256];
float complex hb_samples2[256];


// channel resamp arrays
//float complex ch_resamp_x[8];
float complex ch_resamp_y[512]; //48 is enough for 16.28/100.0
unsigned int ch_ny = 0;


//up-sample voice audio from 8 Ksps  to 16.276 Ksps
float audio_resamp_r           = 2.0345f;   //resampling rate==2.0345 (output/input) to give desired
//float audio_resamp_r           = 6.0f;   //resampling rate==2.0345 (output/input) to give desired
unsigned int audio_resamp_m    = RESAMP_RF_M;     // resampling filter semi-length (filter delay)
float audio_resamp_As          = -60.0f;  // resampling filter stop-band attenuation [dB]
float audio_resamp_bw          = 0.36f;  // resampling filter bandwidth ,  bw of 0.45 takes edge off
unsigned int audio_resamp_npfb = RESAMP_RF_NFILTERS;     // number of filters in bank (timing resolution)

static struct resamp_rf_s audio_rs;
struct resamp_rf_s *audio_resamp_q;
// audio resamp arrays
static float audio_resamp_x[160];
static float audio_resamp_y[160 * 6];
volatile short upsampled[160 * 6];
volatile int upsample_ready;


//post FM demod audio filter before being sent to decoder
static struct firfilt_rf_s audio_fir_s;
static struct firfilt_rf_s *audio_fir;
static float audio_fir_bw = ( POST_FM_FLT_BW / DECODER_FS );
static float audio_fir_as = -60.0f;
static int audio_fir_len = 4.0;
static float fm_filtered;

//RF channel fir
//static struct firfilt_cf_s channel_fir_s;
//static struct firfilt_cf_s *channel_fir;
//static float channel_fir_bw = (RF_CHANNEL_BW*2)/SAMPLE_FREQ;
//static float channel_fir_as = -80.0f;
//static int channel_fir_len = 160;
//static float complex channel_output;

struct nco_cf_s *nco;
struct freqmod_s *freqmod;

struct eqlms_rf_s *eqlms;
// options
unsigned int eqlms_k = 10;         // filter samples/symbol
unsigned int eqlms_m = 3;         // filter semi-length (symbols)
float eqlms_beta = 0.3f;          // filter excess bandwidth factor
float eqlms_mu = 0.200f;          // LMS equalizer learning rate
float eqlms_d[1];
float eqlms_dhat[1];

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
float get_real_amp( void )
{
  return real_amp;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
float atan2f( float y, float x )
{
  //http://pubs.opengroup.org/onlinepubs/009695399/functions/atan2.html
  //Volkan SALMA

  const float ONEQTR_PI = M_PI / 4.0;
  const float THRQTR_PI = 3.0 * M_PI / 4.0;
  float r, angle;
  float abs_y = fabs( y ) + 1e-10f;    // kludge to prevent 0/0 condition
  if( x < 0.0f ) {
    r = ( x + abs_y ) / ( abs_y - x );
    angle = THRQTR_PI;
  } else {
    r = ( x - abs_y ) / ( x + abs_y );
    angle = ONEQTR_PI;
  }
  angle += ( 0.1963f * r * r - 0.9817f ) * r;
  if( y < 0.0f )
    return( -angle );     // negate if in quad III or IV
  else
    return( angle );


}


/**
  * @brief  The application entry point.
  *
  * @retval None
  */
int main( void )
{
  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  HAL_Delay( 1000 );

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */


  MPU_Config();

  SCB_EnableICache();
  //SCB_DisableICache();

  /* USER CODE END 1 */

  /* Enable D-Cache-------------------------------------------------------------*/
  SCB_EnableDCache();
  //SCB_DisableDCache();

  /* MCU Configuration----------------------------------------------------------*/


  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  //MX_GPIO_Init();
  /* Initialize all configured peripherals */
  //MX_ADC1_Init();
  //MX_ADC3_Init();
  //MX_SDMMC2_SD_Init();


  /* Configure the system clock */
  SystemClock_Config();
  /* USER CODE BEGIN 1 */

  //HAL_Delay(10);
  //MX_SPI3_Init();  //must be after fpga bit-bang init
  init_globals(); //this needs to be done before uart configuration  (baud rate) and networking configuration, etc


  MX_USART3_UART_Init();
  //

  MX_LWIP_Init();

  /* USER CODE BEGIN 2 */
  // create frame generator with default properties

  printf( "\r\n\r\nInstruction Cache Enabled" );
  printf( "\r\nData Cache Enabled" );
  printf( "\r\nHAL Initialized" );

  if( set_mco2 ) printf( "\r\nconfigure MCO2 for 200 MHz" );

  clk_mhz = SystemCoreClock / 1e6;
  printf( "\r\nSTM32H743 Running @ %lu MHz", clk_mhz );


  tcp_iperf_init();
  printf( "\r\niperf3 server started on port 5201" );


  mbelib_test_init();

  //demodulate
  freqdem_init( 0.15f );

  //modulate
  //freqmod = freqmod_create( 0.15f );

  int ii;

  p25_udp_init();
  telnet_init();
  rtl_tcp_driver_init();


  q = symsync_create_rnyquist( &symsync, symsync_ftype, symsync_k, symsync_m, symsync_beta, symsync_npfb );
  ch_resamp_q = resamp_cf_create( &ch_rs, ch_resamp_r, ch_resamp_m, ch_resamp_bw, ch_resamp_As, ch_resamp_npfb );
  audio_resamp_q = resamp_rf_create( &audio_rs, audio_resamp_r, audio_resamp_m, audio_resamp_bw, audio_resamp_As, audio_resamp_npfb );
  audio_fir = firfilt_rf_create_kaiser( &audio_fir_s, audio_fir_len, audio_fir_bw, audio_fir_as, 0.0f );

  //channel_fir = firfilt_cf_create_kaiser( &channel_fir_s, channel_fir_len, channel_fir_bw, channel_fir_as, 0.0f );
  nco = nco_create( LIQUID_NCO );
  nco_set_frequency( nco, 0.25f * M_PI );

  halfband_q = resamp2_create( &halfband_s, half_band_m, 0.0f, half_band_as );

  //eqlms = eqlms_rf_create_rnyquist( LIQUID_FIRFILT_RRC, eqlms_k, eqlms_m, eqlms_beta, 0 );
  //eqlms_rf_set_bw( eqlms, eqlms_mu );

  int16_t *sptr;
  int ii_len;

  int free = _mem_free();
  printf( "\r\nheap mem free %d bytes\r\n", free );


  MX_SPI3_Init();  //must be after fpga bit-bang init
  init_flash_s08();

  init_dc_correction();




  ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  //  main loop
  ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  while( 1 ) {

    MX_LWIP_Process();
    //HAL_GPIO_WritePin( GPIOE, (GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4), GPIO_PIN_RESET );

    if( in_buffer_len > 0 && in_buffer_len % 2 == 0 ) {


      if( in_buffer_len > 0 ) {
        if( config->scanner_mode == 1 && stddev_n < N_SAMPLES - 1 ) calc_stddev( ( int16_t * ) in_buffer, in_buffer_len );
      }

      ii_len = in_buffer_len;
      in_buffer_len -= ii_len;
      memcpy( in_buffer_tmp, in_buffer, ii_len );

      sptr = ( int16_t * ) &in_buffer_tmp[0];

      for( ii = 0; ii < ii_len / 2; ii++ ) {
        if( found_signal ) process_incoming_samples( ( int16_t ) *sptr++ );
        MX_LWIP_Process();
        led_tick();
      }

    }
    main_tick();
    command_tick();

    current_rssi = ( int ) get_rssi();

  }

}
/*
//////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////
void test_resamp(void) {
    // options
    float r_r           = 1.1f;   // resampling rate (output/input)
    unsigned int r_m    = 13;     // resampling filter semi-length (filter delay)
    float r_As          = 60.0f;  // resampling filter stop-band attenuation [dB]
    float r_bw          = 0.45f;  // resampling filter bandwidth
    unsigned int r_npfb = 32;     // number of filters in bank (timing resolution)
    unsigned int r_n    = 400;    // number of input samples
    float r_fc          = 0.044f; // complex sinusoid frequency

    static struct resamp_cf_s rs;
    static struct resamp_cf_s *resamp_q;

    // arrays
    float complex r_x[413+1];
    float complex r_y[504+1];


    unsigned int i;

    // number of input samples (zero-padded)
    unsigned int nx = r_n + r_m;

    // output buffer with extra padding for good measure
    unsigned int y_len = (unsigned int) ceilf(1.1 * nx * r_r) + 4;

    printf("\r\nnx: %d", nx);
    printf("\r\nylen: %d", y_len);

    resamp_q = resamp_cf_create(&rs, r_r,r_m,r_bw,r_As,r_npfb);


    // generate input signal
    float wsum = 0.0f;
    for (i=0; i<nx; i++) {
        // compute window
        float w = i < r_n ? kaiser(i, r_n, 10.0f, 0.0f) : 0.0f;

        // apply window to complex sinusoid
        r_x[i] = cexpf(_Complex_I*2*M_PI*r_fc*i) * w;

        // accumulate window
        wsum += w;

    }


    // resample
    unsigned int ny=0;
    resamp_cf_execute_block(resamp_q, r_x, nx, r_y, &ny);


    for (i=0; i<ny; i++) {
      printf("\r\n%f, %f", creal(r_y[i]), cimag(r_y[i]) );
      MX_LWIP_Process();
    }
}
*/

/////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////
void main_tick()
{

  current_time = HAL_GetTick();
  if( ( current_time - telnet_tick_time ) > 50 ) {
    telnet_tick();
    telnet_tick_time = current_time;
  }

  current_time = HAL_GetTick();
  if( current_time - second_tick > 1000 ) {
    second_tick = current_time;
    uptime_sec++;
    //printf("\r\nsamples %d", sample_cnt);
    sample_cnt = 0;

    //show_button_reg();
  }

  MX_LWIP_Process();

}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////
void led_tick()
{

  current_time = HAL_GetTick();

  if( ( current_time - led_time ) > 1 ) {


    p25_net_tick();

    audio_frames_done = 0;
    led_time = current_time;

    if( do_silent_frame++ > 3 ) {
      do_silent_frame = 0;
      //flush_audio();
    }


    //blink the on-board red/blue leds every 10ms so we know the code is running
    led_state ^= 1;
    if( led_state ) {
      //HAL_GPIO_WritePin( GPIOE, (GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4), GPIO_PIN_SET );
    } else {
      //HAL_GPIO_WritePin( GPIOE, (GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4), GPIO_PIN_RESET );
    }

    led_time = current_time;

  }
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config( void )
{

  RCC_OscInitTypeDef RCC_OscInitStruct;
  RCC_ClkInitTypeDef RCC_ClkInitStruct;
  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct;


  ////////////////////////////////////////////////////////////////////////////////////////////////
  //  NOTE: Not sure why, but 100 Mbps Ethernet only works at SCALE3 (default/lowest voltage)
  //  when running from HSE.  100 Mbps does work when running 64 MHz from HSI at SCALE1
  ////////////////////////////////////////////////////////////////////////////////////////////////
  /**Supply configuration update enable
  */
  MODIFY_REG( PWR->CR3, PWR_CR3_SCUEN, 0 );

  __HAL_PWR_VOLTAGESCALING_CONFIG( PWR_REGULATOR_VOLTAGE_SCALE1 );

  while( ( PWR->D3CR & ( PWR_D3CR_VOSRDY ) ) != PWR_D3CR_VOSRDY ) {

  }
  HAL_Delay( 10 );



  __HAL_RCC_PLL_PLLSOURCE_CONFIG( RCC_PLLSOURCE_HSE );
  HAL_Delay( 10 );


  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI | RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSIState = RCC_HSI_DIV1;
  RCC_OscInitStruct.HSICalibrationValue = 16;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 1;
  RCC_OscInitStruct.PLL.PLLN = 100;
  RCC_OscInitStruct.PLL.PLLP = 2;
  RCC_OscInitStruct.PLL.PLLQ = 2;
  RCC_OscInitStruct.PLL.PLLR = 2;
  RCC_OscInitStruct.PLL.PLLRGE = RCC_PLL1VCIRANGE_3;
  RCC_OscInitStruct.PLL.PLLVCOSEL = RCC_PLL1VCOWIDE;
  RCC_OscInitStruct.PLL.PLLFRACN = 0;
  if( HAL_RCC_OscConfig( &RCC_OscInitStruct ) != HAL_OK ) {
    _Error_Handler( __FILE__, __LINE__ );
  }
  HAL_Delay( 10 );

  /**Initializes the CPU, AHB and APB busses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
                                | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2
                                | RCC_CLOCKTYPE_D3PCLK1 | RCC_CLOCKTYPE_D1PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.SYSCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB3CLKDivider = RCC_APB3_DIV2;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_APB1_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_APB2_DIV2;
  RCC_ClkInitStruct.APB4CLKDivider = RCC_APB4_DIV2;

  if( HAL_RCC_ClockConfig( &RCC_ClkInitStruct, FLASH_LATENCY_2 ) != HAL_OK ) {
    _Error_Handler( __FILE__, __LINE__ );
  }
  HAL_Delay( 10 );

  PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_USART3 | RCC_PERIPHCLK_SPI3
      | RCC_PERIPHCLK_SDMMC | RCC_PERIPHCLK_ADC
      | RCC_PERIPHCLK_FMC | RCC_PERIPHCLK_CKPER;
  PeriphClkInitStruct.PLL2.PLL2M = 1;
  PeriphClkInitStruct.PLL2.PLL2N = 19;
  PeriphClkInitStruct.PLL2.PLL2P = 1;
  PeriphClkInitStruct.PLL2.PLL2Q = 2;
  PeriphClkInitStruct.PLL2.PLL2R = 2;
  PeriphClkInitStruct.PLL2.PLL2RGE = RCC_PLL2VCIRANGE_3;
  PeriphClkInitStruct.PLL2.PLL2VCOSEL = RCC_PLL2VCOMEDIUM;
  PeriphClkInitStruct.PLL2.PLL2FRACN = 0;
  PeriphClkInitStruct.FmcClockSelection = RCC_FMCCLKSOURCE_D1HCLK;
  PeriphClkInitStruct.SdmmcClockSelection = RCC_SDMMCCLKSOURCE_PLL;
  PeriphClkInitStruct.CkperClockSelection = RCC_CLKPSOURCE_HSI;
  PeriphClkInitStruct.Spi123ClockSelection = RCC_SPI123CLKSOURCE_CLKP;
  PeriphClkInitStruct.Usart234578ClockSelection = RCC_USART234578CLKSOURCE_D2PCLK1;
  PeriphClkInitStruct.AdcClockSelection = RCC_ADCCLKSOURCE_PLL2;
  if( HAL_RCCEx_PeriphCLKConfig( &PeriphClkInitStruct ) != HAL_OK ) {
    _Error_Handler( __FILE__, __LINE__ );
  }
  HAL_Delay( 10 );

  //HAL_RCC_MCOConfig( RCC_MCO2, RCC_MCO2SOURCE_SYSCLK, RCC_MCODIV_2 );
  //HAL_Delay( 10 );
  //set_mco2 = 1;


  /**Configure the Systick interrupt time
  */
  HAL_SYSTICK_Config( SystemCoreClock / 1000 );

  /**Configure the Systick
  */
  HAL_SYSTICK_CLKSourceConfig( SYSTICK_CLKSOURCE_HCLK );

  /* SysTick_IRQn interrupt configuration */
  HAL_NVIC_SetPriority( SysTick_IRQn, 0, 0 );
}


/* USART3 init function */
static void MX_USART3_UART_Init( void )
{

  huart3.Instance = USART3;
  huart3.Init.BaudRate = config->uart3_baudrate;
  huart3.Init.WordLength = UART_WORDLENGTH_8B;
  huart3.Init.StopBits = UART_STOPBITS_1;
  huart3.Init.Parity = UART_PARITY_NONE;
  huart3.Init.Mode = UART_MODE_TX_RX;
  huart3.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart3.Init.OverSampling = UART_OVERSAMPLING_16;
  huart3.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart3.Init.Prescaler = UART_PRESCALER_DIV1;
  huart3.Init.FIFOMode = UART_FIFOMODE_DISABLE;
  huart3.Init.TXFIFOThreshold = UART_TXFIFO_THRESHOLD_1_8;
  huart3.Init.RXFIFOThreshold = UART_RXFIFO_THRESHOLD_1_8;
  huart3.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if( HAL_UART_Init( &huart3 ) != HAL_OK ) {
    _Error_Handler( __FILE__, __LINE__ );
  }

}

/** Configure pins as
        * Analog
        * Input
        * Output
        * EVENT_OUT
        * EXTI
     PA8   ------> USB_OTG_FS_SOF
     PA11   ------> USB_OTG_FS_DM
     PA12   ------> USB_OTG_FS_DP
*/
static void MX_GPIO_Init( void )
{

  GPIO_InitTypeDef GPIO_InitStruct;

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOE_CLK_ENABLE();
  __HAL_RCC_GPIOF_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOG_CLK_ENABLE();


  //LEDs

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin( GPIOE, GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4, GPIO_PIN_SET );
  //HAL_GPIO_WritePin( GPIOE, GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4, GPIO_PIN_RESET );

  GPIO_InitStruct.Pin = GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init( GPIOE, &GPIO_InitStruct );



  //display pins, all output
  //PG0 = DISP_RS
  //PG1 = DISP_SI
  //PG2 = DISP_CSB
  //PG3 = DISP_SCL
  //PG4 = DISP_RST
  GPIO_InitStruct.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init( GPIOG, &GPIO_InitStruct );

  HAL_GPIO_WritePin( GPIOG, GPIO_PIN_0 | GPIO_PIN_1, GPIO_PIN_SET ); //at cs, tuner cs

  //button pins, all input
  //up, down, left, right, sel, mode
  GPIO_InitStruct.Pin = GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_12 | GPIO_PIN_13;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init( GPIOF, &GPIO_InitStruct );


  //MCO2 output
  /*Configure GPIO pin : PC9 */
  GPIO_InitStruct.Pin = GPIO_PIN_9;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF0_MCO;
  HAL_GPIO_Init( GPIOC, &GPIO_InitStruct );

}

/* USER CODE BEGIN 4 */
/**
  * @brief  Configure the MPU attributes
  * @param  None
  * @retval None
  */
static void MPU_Config( void )
{
  MPU_Region_InitTypeDef MPU_InitStruct;

  /* Disable the MPU */
  HAL_MPU_Disable();

  /* Configure the MPU attributes as Device not cacheable
     for ETH DMA descriptors */
  MPU_InitStruct.Enable = MPU_REGION_ENABLE;
  MPU_InitStruct.BaseAddress = 0x30040000;
  MPU_InitStruct.Size = MPU_REGION_SIZE_256B;
  MPU_InitStruct.AccessPermission = MPU_REGION_FULL_ACCESS;
  MPU_InitStruct.IsBufferable = MPU_ACCESS_BUFFERABLE;
  MPU_InitStruct.IsCacheable = MPU_ACCESS_NOT_CACHEABLE;
  MPU_InitStruct.IsShareable = MPU_ACCESS_NOT_SHAREABLE;
  MPU_InitStruct.Number = MPU_REGION_NUMBER0;
  MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL0;
  MPU_InitStruct.SubRegionDisable = 0x00;
  MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_ENABLE;

  HAL_MPU_ConfigRegion( &MPU_InitStruct );

  /* Configure the MPU attributes as Cacheable write through
     for LwIP RAM heap which contains the Tx buffers */
  MPU_InitStruct.Enable = MPU_REGION_ENABLE;
  MPU_InitStruct.BaseAddress = 0x30044000; //also defined in lwipopts.h as #define LWIP_RAM_HEAP_POINTER    (0x30044000)
  MPU_InitStruct.Size = MPU_REGION_SIZE_16KB;
  MPU_InitStruct.AccessPermission = MPU_REGION_FULL_ACCESS;
  MPU_InitStruct.IsBufferable = MPU_ACCESS_NOT_BUFFERABLE;
  MPU_InitStruct.IsCacheable = MPU_ACCESS_CACHEABLE;
  MPU_InitStruct.IsShareable = MPU_ACCESS_NOT_SHAREABLE;
  MPU_InitStruct.Number = MPU_REGION_NUMBER1;
  MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL0;
  MPU_InitStruct.SubRegionDisable = 0x00;
  MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_ENABLE;

  HAL_MPU_ConfigRegion( &MPU_InitStruct );


  //FMC / FPGA address space,  16-bit
  MPU_InitStruct.Enable = MPU_REGION_ENABLE;
  MPU_InitStruct.BaseAddress = 0x60000000;
  MPU_InitStruct.Size = MPU_REGION_SIZE_64MB;
  MPU_InitStruct.AccessPermission = MPU_REGION_FULL_ACCESS;
  MPU_InitStruct.IsBufferable = MPU_ACCESS_NOT_BUFFERABLE;
  MPU_InitStruct.IsCacheable = MPU_ACCESS_NOT_CACHEABLE;
  MPU_InitStruct.IsShareable = MPU_ACCESS_NOT_SHAREABLE;
  MPU_InitStruct.Number = MPU_REGION_NUMBER2;
  MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL0;
  MPU_InitStruct.SubRegionDisable = 0x00;
  MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_DISABLE;

  HAL_MPU_ConfigRegion( &MPU_InitStruct );

  /* Enable the MPU */
  HAL_MPU_Enable( MPU_PRIVILEGED_DEFAULT );
}
/* ADC1 init function */
static void MX_ADC1_Init( void )
{

  ADC_MultiModeTypeDef multimode;
  ADC_ChannelConfTypeDef sConfig;

  /**Common config
  */
  hadc1.Instance = ADC1;
  hadc1.Init.ClockPrescaler = ADC_CLOCK_ASYNC_DIV6;
  hadc1.Init.Resolution = ADC_RESOLUTION_16B;
  hadc1.Init.ScanConvMode = ADC_SCAN_DISABLE;
  hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  hadc1.Init.LowPowerAutoWait = DISABLE;
  hadc1.Init.ContinuousConvMode = DISABLE;
  hadc1.Init.NbrOfConversion = 1;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.NbrOfDiscConversion = 1;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc1.Init.ConversionDataManagement = ADC_CONVERSIONDATA_DR;
  hadc1.Init.Overrun = ADC_OVR_DATA_PRESERVED;
  hadc1.Init.LeftBitShift = ADC_LEFTBITSHIFT_NONE;
  hadc1.Init.BoostMode = ENABLE;
  hadc1.Init.OversamplingMode = DISABLE;
  if( HAL_ADC_Init( &hadc1 ) != HAL_OK ) {
    _Error_Handler( __FILE__, __LINE__ );
  }

  /**Configure the ADC multi-mode
  */
  multimode.Mode = ADC_MODE_INDEPENDENT;
  if( HAL_ADCEx_MultiModeConfigChannel( &hadc1, &multimode ) != HAL_OK ) {
    _Error_Handler( __FILE__, __LINE__ );
  }

  /**Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_19;
  sConfig.Rank = ADC_REGULAR_RANK_1;
  sConfig.SamplingTime = ADC_SAMPLETIME_1CYCLE_5;
  sConfig.SingleDiff = ADC_SINGLE_ENDED;
  sConfig.OffsetNumber = ADC_OFFSET_NONE;
  sConfig.Offset = 0;
  if( HAL_ADC_ConfigChannel( &hadc1, &sConfig ) != HAL_OK ) {
    _Error_Handler( __FILE__, __LINE__ );
  }

}

/* ADC3 init function */
static void MX_ADC3_Init( void )
{

  ADC_ChannelConfTypeDef sConfig;

  /**Common config
  */
  hadc3.Instance = ADC3;
  hadc3.Init.ClockPrescaler = ADC_CLOCK_ASYNC_DIV6;
  hadc3.Init.Resolution = ADC_RESOLUTION_16B;
  hadc3.Init.ScanConvMode = ADC_SCAN_DISABLE;
  hadc3.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  hadc3.Init.LowPowerAutoWait = DISABLE;
  hadc3.Init.ContinuousConvMode = DISABLE;
  hadc3.Init.NbrOfConversion = 1;
  hadc3.Init.DiscontinuousConvMode = DISABLE;
  hadc3.Init.NbrOfDiscConversion = 1;
  hadc3.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc3.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc3.Init.ConversionDataManagement = ADC_CONVERSIONDATA_DR;
  hadc3.Init.Overrun = ADC_OVR_DATA_PRESERVED;
  hadc3.Init.LeftBitShift = ADC_LEFTBITSHIFT_NONE;
  hadc3.Init.BoostMode = ENABLE;
  hadc3.Init.OversamplingMode = DISABLE;
  if( HAL_ADC_Init( &hadc3 ) != HAL_OK ) {
    _Error_Handler( __FILE__, __LINE__ );
  }

  /**Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_1;
  sConfig.Rank = ADC_REGULAR_RANK_1;
  sConfig.SamplingTime = ADC_SAMPLETIME_1CYCLE_5;
  sConfig.SingleDiff = ADC_SINGLE_ENDED;
  sConfig.OffsetNumber = ADC_OFFSET_NONE;
  sConfig.Offset = 0;
  if( HAL_ADC_ConfigChannel( &hadc3, &sConfig ) != HAL_OK ) {
    _Error_Handler( __FILE__, __LINE__ );
  }

}

/* SDMMC2 init function */
static void MX_SDMMC2_SD_Init( void )
{

  hsd2.Instance = SDMMC2;
  hsd2.Init.ClockEdge = SDMMC_CLOCK_EDGE_RISING;
  hsd2.Init.ClockPowerSave = SDMMC_CLOCK_POWER_SAVE_DISABLE;
  hsd2.Init.BusWide = SDMMC_BUS_WIDE_1B;
  hsd2.Init.HardwareFlowControl = SDMMC_HARDWARE_FLOW_CONTROL_DISABLE;
  hsd2.Init.ClockDiv = 0;
  if( HAL_SD_Init( &hsd2 ) != HAL_OK ) {
    _Error_Handler( __FILE__, __LINE__ );
  }

}

/* SPI3 init function */
void MX_SPI3_Init( void )
{

  /* SPI3 parameter configuration*/

  hspi3.Instance = SPI3;
  hspi3.Init.Mode = SPI_MODE_MASTER;
  hspi3.Init.Direction = SPI_DIRECTION_2LINES;
  hspi3.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi3.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi3.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi3.Init.NSS = SPI_NSS_SOFT;
  hspi3.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_4;
  hspi3.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi3.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi3.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi3.Init.CRCPolynomial = 7;
  hspi3.Init.NSSPMode = SPI_NSS_PULSE_DISABLE;
  hspi3.Init.NSSPolarity = SPI_NSS_POLARITY_LOW;
  hspi3.Init.FifoThreshold = SPI_FIFO_THRESHOLD_01DATA;
  hspi3.Init.TxCRCInitializationPattern = SPI_CRC_INITIALIZATION_ALL_ZERO_PATTERN;
  hspi3.Init.RxCRCInitializationPattern = SPI_CRC_INITIALIZATION_ALL_ZERO_PATTERN;
  hspi3.Init.MasterSSIdleness = SPI_MASTER_SS_IDLENESS_00CYCLE;
  hspi3.Init.MasterInterDataIdleness = SPI_MASTER_INTERDATA_IDLENESS_00CYCLE;
  hspi3.Init.MasterReceiverAutoSusp = SPI_MASTER_RX_AUTOSUSP_DISABLE;
  hspi3.Init.MasterKeepIOState = SPI_MASTER_KEEP_IO_STATE_ENABLE;
  hspi3.Init.IOSwap = SPI_IO_SWAP_DISABLE;

  __HAL_RCC_SPI3_FORCE_RESET();
  __NOP();
  __HAL_RCC_SPI3_RELEASE_RESET();

  if( HAL_SPI_Init( &hspi3 ) != HAL_OK ) {
    _Error_Handler( __FILE__, __LINE__ );
  }

  //hspi3.Instance->CR1 |=  SPI_CR1_SPE;

}

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @param  file: The file name as string.
  * @param  line: The line in file as a number.
  * @retval None
  */
void _Error_Handler( char *file, int line )
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  printf( "\r\n_Error_Handler: file %s, line %d", file, line );
  while( 1 ) {
    main_tick();
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed( uint8_t *file, uint32_t line )
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

//////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////
void udp_data_rx( void *arg, struct udp_pcb *pcb, struct pbuf *p, struct ip4_addr *addr, uint16_t port )
{

  int i;
  uint8_t *ptr;

  if( p != NULL ) {

    struct pbuf *q = p;
    int len = q->len;
    ptr = q->payload;

    if( p->tot_len > 10 && ( in_buffer_len + p->tot_len ) <  sizeof( in_buffer ) ) {

      while( q ) {
        for( i = 0; i < len; i++ ) {
          if( in_buffer_len < sizeof( in_buffer ) ) in_buffer[in_buffer_len++] = *ptr++;
        }

        q = q->next;
        if( q != NULL ) {
          len = q->len;
          ptr = q->payload;
        }
      }

    }

    pbuf_free( p );


  }
}


////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////
void p25_udp_init()
{
  IP4_ADDR( &udp_saddr, udp_send_addr[0], udp_send_addr[1], udp_send_addr[2], udp_send_addr[3] );

  udp_data_pcb = udp_new();
  if( udp_data_pcb == NULL ) {
    return;
  }
  //}
  udp_bind( udp_data_pcb, IP_ADDR_ANY, 8889 );
  udp_recv( udp_data_pcb, ( void * ) udp_data_rx, NULL );

}

///////////////////////////////////////////////////////////////////////////////////////////////
// up-sample voice audio from 8 Ksps to 16.276 Ksps using poly-phase fir filter
// in other words, for every 160 samples of incoming voice, 325.52 samples are generated.
//
// up-sample rate is 2.0345f:   resampling rate==2.0345 (output/input) to give desired
// rate for FPGA clocks of 16.276 Ksps for audio.
//
// FPGA master_clock is 100 MHz, audio mclk/sclk ratio is 16x
// audio mclk = master_clock/12, audio sclk is master_clock/192
///////////////////////////////////////////////////////////////////////////////////////////////
int upsample_audio( uint8_t *buffer, int len )
{

  if( len > 320 ) return 0; //not audio

#if 1

  int i;
  short *ptr = ( short * ) buffer;
  for( i = 0; i < len / 2; i++ ) {
    audio_resamp_x[i] = ( float ) * ptr++;
  }

  //upsample from 8khz to 16.xxkhz
  unsigned int a_ny = 0;
  resamp_rf_execute_block( audio_resamp_q, audio_resamp_x, len / 2, audio_resamp_y, &a_ny );

  //printf("\r\nupsampled %d", a_ny);

  for( i = 0; i < a_ny; i++ ) {
    //if( isnan( audio_resamp_y[i] ) ) printf("\r\naudio resampler NAN error"); //not the issue
    upsampled[i] = ( short ) audio_resamp_y[i];
  }

  upsample_ready = 1;

  return a_ny;
#endif
}

////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////
void udp_send_data( uint8_t *buffer, int len )
{

  return;




#if 1
  //48Ksps audio after being upsampled
  char *aptr = ( char * ) upsampled;
  p = pbuf_alloc( PBUF_TRANSPORT, len * 6, PBUF_RAM );
  if( p != NULL ) {
    memcpy( p->payload, aptr, len * 6 );
    udp_sendto( udp_data_pcb, p, &udp_saddr, 8889 );
    pbuf_free( p );
  }
#else
  //8Ksps audio directly from MBE synthesizer
  p = pbuf_alloc( PBUF_TRANSPORT, len, PBUF_RAM );
  if( p != NULL ) {
    memcpy( p->payload, buffer, len );
    udp_sendto( udp_data_pcb, p, &udp_saddr, 9888 );
    pbuf_free( p );
  }
#endif
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////
void process_incoming_rf( void )
{
  volatile int _rfi;
  volatile int8_t *sptr = ( int8_t * ) iq_data;
  //volatile int idx=0;
  short audio_sample;
  char *p;
  real_amp = 0.0f;

  int8_t r1;
  int8_t r2;

  int i;
  volatile struct pbuf *pb;

  sptr = ( int8_t * ) iq_data;

  float _I;
  float _Q;

  //gather the samples in float format for dsp chain
  for( _rfi = 0; _rfi < 256; _rfi++ ) {
    //samples[_rfi] = ( float complex )( ( (((float) *sptr++)+config->i_off) + _Complex_I * (((float) *sptr++)+config->q_off) ) );
    _I = ( float ) * sptr++;
    _Q = ( float ) * sptr++;

    //_I += config->i_off;  //no longer needed
    //_Q += config->q_off;

    _I *= -1.0; //mirror frequency spectrum due to external mixer
    _Q *= -1.0;

    //dc offset correction
    d_avg_i = d_avg_i * d_beta_i + _I * d_alpha_i;
    _I -= d_avg_i;

    d_avg_q = d_avg_q * d_beta_q + _Q * d_alpha_q;
    _Q -= d_avg_q;


    samples[_rfi] = _I + _Complex_I * _Q;

    if( do_low_if_mix ) { //down-mix from 12.5KHz offset  (tuned 12.5khz lower than desired receive channel
      nco_step( nco );
      nco_mix_down( nco, samples[_rfi], &samples[_rfi] );
    }
  }

  if( config->sa_mode != 0 ) {

    if( config->sa_mode == 1 ) {
      decimate = 8;
      do_filter_decimate_8( samples );

      sptr = ( int8_t * ) iq_data;  //use decimated by 8 output
      for( _rfi = 0; _rfi < 256 / decimate; _rfi++ ) {
        *sptr++ = ( int8_t )( ( iout[_rfi] ) / 3.0 ); //divide by 16 to compensate for gain in decimation filter and 8-bit output
        *sptr++ = ( int8_t )( ( qout[_rfi] ) / 3.0 );
      }
    } else if( config->sa_mode == 2 ) {
      decimate = 2;
      do_fircomp_dec2_stage1( samples );

      for( _rfi = 0; _rfi < 256 / decimate; _rfi++ ) {
        samples[_rfi] = ( float complex )( ( ( ( float ) iout[_rfi] ) + _Complex_I * ( ( float ) qout[_rfi] ) ) );
      }
      do_fircomp_dec2_stage2( samples );

      decimate = 4;
      sptr = ( int8_t * ) iq_data;
      for( _rfi = 0; _rfi < 256 / decimate; _rfi++ ) {
        *sptr++ = ( int8_t )( ( iout[_rfi] ) ); //divide by 16 to compensate for gain in decimation filter and 8-bit output
        *sptr++ = ( int8_t )( ( qout[_rfi] ) );
      }
    } else if( config->sa_mode == 3 ) {
      decimate = 32;
      do_filter_decimate_32( samples );
      sptr = ( int8_t * ) iq_data;  //use decimated by 8 output
      for( _rfi = 0; _rfi < 256 / decimate; _rfi++ ) {
        *sptr++ = ( int8_t )( ( iout[_rfi] ) / 3.0 ); //divide by 16 to compensate for gain in decimation filter and 8-bit output
        *sptr++ = ( int8_t )( ( qout[_rfi] ) / 3.0 );
      }
      sptr = ( int8_t * ) iq_data;

    } else if( config->sa_mode == 4 ) {
      return;
    }

    sptr = ( int8_t * ) iq_data;

    pb = pbuf_alloc( PBUF_TRANSPORT, ( 256 * 2 ) / decimate, PBUF_RAM );
    if( pb != NULL ) {

      memcpy( pb->payload, ( uint8_t * ) sptr, ( 256 * 2 ) / decimate );
      udp_sendto( udp_data_pcb, pb, &udp_saddr, 8889 );
      pbuf_free( pb );
    }
    return;
  } else {
    decimate = 8;
    audio_out_level = 10000.0f;
  }

  do_filter_decimate_8( samples ); //decimate by 8 FIR
  for( _rfi = 0; _rfi < 256 / decimate; _rfi++ ) {
    samples[_rfi] = ( float complex )( ( ( ( ( float ) iout[_rfi] ) / 16.0 ) + _Complex_I * ( ( ( float ) qout[_rfi] ) / 16.0 ) ) );
  }

  freqdem_demodulate( samples, 256 / decimate, fm_out ); //use fast ATAN method for demod

  //resample back up to 48kHz for p25 sync/decoder
  for( _rfi = 0; _rfi < 256 / decimate; _rfi++ ) {
    samples[_rfi] = ( float complex )( ( ( ( ( float ) fm_out[_rfi] ) ) + _Complex_I * ( ( ( float ) 0.0f ) ) ) );
  }
  resamp_cf_execute_block( ch_resamp_q, samples, 256 / decimate, ch_resamp_y, &ch_ny );

  uint8_t *inb = ( uint8_t * ) &in_buffer[in_buffer_len];

  for( _rfi = 0; _rfi < ch_ny; _rfi++ ) {

    //audio fir for post-FM demod filtering
    firfilt_rf_push( audio_fir, creal( ch_resamp_y[_rfi] ) );
    firfilt_rf_execute( audio_fir, &fm_filtered );

    audio_sample = ( short )( fm_filtered * audio_out_level );

    p = ( char * ) &audio_sample;

    if( in_buffer_len < sizeof( in_buffer ) - 1 ) {

      //squelch
      if( current_rssi > config->squelch ) {
        in_buffer[in_buffer_len++] = *p++;
        in_buffer[in_buffer_len++] = *p++;
      } else {
        in_buffer[in_buffer_len++] = 0;
        in_buffer[in_buffer_len++] = 0;
      }
    }
  }

  //send the final fm-demod/filtered audio product over udp
  pb = pbuf_alloc( PBUF_TRANSPORT, ch_ny * 2, PBUF_RAM );
  if( pb != NULL ) {

    memcpy( pb->payload, ( uint8_t * ) inb, ch_ny * 2 );
    udp_sendto( udp_data_pcb, pb, &udp_saddr, 8889 );
    pbuf_free( pb );
  }

}


//////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////
void process_incoming_samples( int16_t val )
{
  sample_cnt++;

  //if(sample_cnt%24000==0) printf("\r\nsample val %d", val);

  symsync_buf_in[0] = ( float ) val;

  did_sym = 0;

#if 0
  eqlms_rf_push( eqlms, ( float ) symsync_buf_in[0] );
  eqlms_rf_execute( eqlms, &eqlms_dhat[0] );
#endif


  //sync to the 4-fsk
  symsync_execute( q, symsync_buf_in, 1, symsync_buf_out, &did_sym );

  //decode the symbols
  if( did_sym ) {
#if 0
    if( p25_is_synced() ) {
      float sym = p25_decode( eqlms_dhat[0], q );

      eqlms_rf_step( eqlms, symsync_buf_out[0], eqlms_dhat[0] ); //adjust coefficients
      //printf("\r\neqlms,%f, %f", symsync_buf_out[0], eqlms_dhat[0]);
    } else {
      p25_decode( symsync_buf_out[0], q );
    }
#endif

#if 1
    p25_decode( symsync_buf_out[0], q );
#endif
  }

}

#if 0
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void test_freqmod()
{
  // options
  float        kf          = 0.1f;    // modulation factor
  unsigned int num_samples = 1024;    // number of samples
  float        SNRdB       = 30.0f;   // signal-to-noise ratio [dB]

  unsigned int i;
  float         m[1024];       // message signal
  float complex r[1024];       // received signal (complex baseband)
  float         y1[1024];       // demodulator output
  float         y2[1024];       // demodulator output

  // generate message signal (sum of sines)
  for( i = 0; i < num_samples; i++ ) {
    m[i] = 0.3f * cosf( 2 * M_PI * 0.013f * i + 0.0f ) +
           0.2f * cosf( 2 * M_PI * 0.021f * i + 0.4f ) +
           0.4f * cosf( 2 * M_PI * 0.037f * i + 1.7f );
  }

  // modulate signal
  freqmod_modulate_block( freqmod, m, num_samples, r );

  // add channel impairments
  float nstd = powf( 10.0f, -SNRdB / 20.0f );
  for( i = 0; i < num_samples; i++ )
    r[i] += nstd * ( randnf() + _Complex_I * randnf() ) * M_SQRT1_2;


  uint32_t start = HAL_GetTick();
  for( i = 0; i < 1000; i++ ) {
    freqdem_demodulate( r, num_samples, y1 );
  }
  uint32_t end = HAL_GetTick();
  printf( "\r\nstd freqdem, run=1000, time %d", end - start );


  start = HAL_GetTick();
  for( i = 0; i < 1000; i++ ) {
    freqdem_demodulate_fast( r, num_samples, y2 );
  }
  end = HAL_GetTick();
  printf( "\r\nfast freqdem, run=1000, time %d", end - start );

  float max_error = 0.0f;
  // demodulate signal
  for( i = 0; i < num_samples; i++ ) {
    if( i > 0 && fabs( y2[i] - y1[i] )  > fabs( max_error ) ) {
      max_error = y2[i] - y1[i];
      printf( "\r\nfreqdem largest error y2-y1 = %f, y1=%f, y2=%f", max_error, y1[i], y2[i] );
    }
  }
  printf( "\r\nfreqdem largest error y2-y1 = %f, y1=%f, y2=%f", max_error, y1[i - 1], y2[i - 1] );

}
#endif

////////////////////////////////////////////////////////////////////////////////
//  noise is around 10,000
////////////////////////////////////////////////////////////////////////////////
void calc_stddev( int16_t *data, int n )
{
  int i = 0;

  while( n-- > 0 && stddev_n < N_SAMPLES ) {
    stddev_in[stddev_n++] = ( float32_t ) data[i++];
  }
  if( stddev_n < N_SAMPLES - 1 ) return;

  stddev_n = 0;

  /*
    arm_std_f32(stddev_in, N_SAMPLES, &stddev_out);
  */

  //if( !found_signal && get_agc_gain() < 21) {  //should be set for each channel
  if( !found_signal && current_rssi > scanner_thresh[scanner_channel] ) {  //should be set for each channel
    found_signal = 1;
    channel_timeout = 10;
  }

  if( channel_timeout > 0 ) {
    channel_timeout--;
    if( channel_timeout == 0 ) {
      found_signal = 0;
      real_amp = 0.0f;
      //p25_reset_stats();
    }
  }

  //if(config->p25_logging>2) printf("\r\nagc gain: %d", get_agc_gain());

  if( config->p25_logging > 2 ) printf( "\r\nchannel %d, rssi: %d", scanner_channel, current_rssi );

  if( !found_signal && channel_timeout == 0 && !p25_is_synced() ) {

    scanner_next_channel();

    channel_timeout = 4;
    did_print_channel = 0;
  } else if( channel_timeout > 0 && found_signal ) {

    if( !did_print_channel && config->p25_logging > 2 ) printf( "\r\nchannel %d < FOUND SIGNAL, rssi: %d", scanner_channel, current_rssi );
    did_print_channel = 1;

    if( p25_is_synced() ) {
      channel_timeout = 60;
    }
  }

}

/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
float get_current_freq()
{
  float f = 857.9625 - ( scanner_freq_p25[scanner_channel] - 954.5 );
  return f;
}

/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
void scanner_next_channel( void )
{

  if( config->scanner_mode == 0 ) return;

  while( 1 ) {
    if( ++scanner_channel == scanner_freqs ) scanner_channel = 0;
    if( scanner_channel_mask[scanner_channel] ) break;
  }
  float new_freq = scanner_freq_p25[scanner_channel];

  at86rf215_set_gain( scanner_gain[scanner_channel] );

  set_frequency_900mhz( new_freq );
  at86rf215_write_single( RF09_CMD, CMD_RF_RX );  //trxoff -> txprep does calibration

  int i;
  int locked = 0;
  for( i = 0; i < 500; i++ ) {
    locked = ( at86rf215_read_single( RF09_PLL ) & 0x02 ) >> 1;
    if( locked ) break;
    delay_us( 1 );
  }
  if( !locked ) printf( "\r\nPLL did not lock!!!!!!!!!!!" );


}
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
void set_channel_timeout_tdulc( void )
{
  channel_timeout = 2;
}
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
void set_channel_timeout_min( void )
{
  channel_timeout = 2;
}
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
void set_channel_timeout_zero( void )
{
  channel_timeout = 0;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
void reset_channel_timeout( void )
{
  channel_timeout = 30 * 6;
}

///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////
int get_current_channel( void )
{
  return ( int ) scanner_channel;
}


///////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
void eval_fir( float *data_in, float *data_out, int len, float *zbuffer, float *coef )
{

  int i;
  int n;
  float sum;

  for( n = 0; n < len; n++ ) {

    for( i = 0; i < FLT_LEN - 1; i++ ) {
      zbuffer[i] = zbuffer[i + 1];
    }
    zbuffer[FLT_LEN - 1] = ( float ) data_in[n];

    sum = 0.0;
    for( i = 0; i < FLT_LEN; i++ ) {
      sum += zbuffer[i] * coef[i];
    }

    data_out[n] = ( float ) sum;
  }
}
/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
void init_dc_correction( void )
{
  ALPHA_D_i = ( 1.0 / ( ( float ) config->srate / 4.0 ) ) * 8.0;
  d_alpha_i = ALPHA_D_i;
  d_beta_i = 1.0 - ALPHA_D_i;
  d_avg_i = 0.0;

  ALPHA_D_q = ( 1.0 / ( ( float ) config->srate / 4.0 ) ) * 8.0;
  d_alpha_q = ALPHA_D_q;
  d_beta_q = 1.0 - ALPHA_D_q;
  d_avg_q = 0.0;
}
