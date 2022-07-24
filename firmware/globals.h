
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



#ifndef __GLOBALS_H_H__
#define __GLOBALS_H_H__

#include "stm32h7xx_hal.h"
#include <stdint.h>
#include "lwip.h"

//#define INVERT_EXT_TUNER 1

#define DEFAULT_P25_NETWORK 0x000 //change this if you don't allow all
#define DEFAULT_P25_RADIO_MANF 0x0000 //change this if you don't allow all
#define CONFIG_ADDRESS 0x000000
#define CONFIG_SIZE (4*1024)

#ifndef M_PI
#define M_PI (3.14159265358979323846)
#endif

#ifndef M_SQRT1_2
#define M_SQRT1_2  0.70710678118654752440  /* 1/sqrt(2) */
#endif

#ifndef M_E
#define M_E    2.7182818284590452354 /* e */
#endif

#define TUNER_CMD_SET_FREQ 0x01
#define TUNER_CMD_GET_FREQ 0x02
#define TUNER_CMD_SET_IF 0x03
#define TUNER_CMD_GET_IF 0x04
#define TUNER_CMD_SET_ANT 0x05
#define TUNER_CMD_GET_ANT 0x06
#define TUNER_CMD_PLL_STATUS 0x07
#define TUNER_CMD_SET_MUTE 0x08
#define TUNER_CMD_GET_LOW_FREQ 0x09
#define TUNER_CMD_GET_HIGH_FREQ 0x0a
#define TUNER_CMD_GET_LOW_IF 0x0b
#define TUNER_CMD_GET_HIGH_IF 0x0c


void init_globals( void );
int read_configuration_from_flash( void );
void write_configuration_to_flash( void );

extern int do_test_mem;
extern int do_test_flash;
extern int do_write_config;
extern int do_read_config;
extern int do_system_reset;
extern int do_iq_mode;
extern int do_rf_tx_test;
extern int do_at_dump;
extern int do_at_scan;
extern int do_at_scan_cont;

typedef struct {
  int audio_on;
  float audio_volume_f;
  int sa_mode;
  int squelch;
  int scanner_mode;
  int srate;
  int rcut;
  int bw;
  int rfgain;
  float i_off;
  float q_off;
  int p25_logging;
  int p25_radiomanf_id;
  int p25_radiomanf_id_allow_all;
  int p25_network_id;
  int p25_network_id_allow_all;
  int uart3_baudrate;
  union {
    char ip_addr[4];
    uint32_t ip_addr32;
  };
} config_t1;

typedef struct {
  union {
    config_t1 *config;
    config_t1 configt1;
    char config_zero[CONFIG_SIZE - 4];
  };
  uint32_t crc32;
} config_t;


extern config_t1 *config;
extern void main_tick( void );
extern void DelayClk3( unsigned long );
extern void delay_ms( int delay );
extern void delay_us( int delay );
extern SPI_HandleTypeDef hspi3;
extern volatile struct udp_pcb *udp_data_pcb;
extern volatile struct ip4_addr udp_saddr;
extern volatile int8_t do_handle_exti1;
extern volatile uint16_t iq_data[512];
extern void process_incoming_rf();
extern volatile int out_s;
extern volatile int out_cnt;
extern volatile int out_e;
extern volatile int is_playing;
void audio_int_config( void );
void rf900_int_config( void );
float get_current_freq( void );
float get_real_amp( void );
void reset_channel_timeout( void );
void set_channel_timeout_min( void );
volatile uint32_t uptime_sec;
void udp_send_response_demod( uint8_t *buffer, int len );
volatile int do_low_if_mix;
volatile int do_freq_offset;
volatile int do_48khz_iq_only;
#endif
