
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




#ifndef __UDP_PORT_H__
#define __UDP_PORT_H__

#include <stdint.h>


enum {
  SDR_COMMAND_NONE,
  SDR_COMMAND_TX,
  SDR_COMMAND_RX,
  SDR_COMMAND_ACK,
  SDR_COMMAND_READ_FLASH,
  SDR_COMMAND_WRITE_FLASH,
  SDR_COMMAND_CRC32_FLASH,
  SDR_COMMAND_BOOT,
  SDR_COMMAND_FLASH
};

enum {
  SDR_OP_MODE_NONE,
  SDR_OP_MODE_RX,
  SDR_OP_MODE_TX,
  SDR_OP_MODE_FIRMWARE_UPDATE,
};

enum {
  STATE_TX_OFF,
  STATE_TX_PREP,
  STATE_TX_ON
};

#define SAMPLE_FORMAT_LE_8BIT_SI    (1<<0)
#define SAMPLE_FORMAT_LE_12BIT_SI   (1<<1)           //12-bit I, 12-bit Q packed into 3-byte chunks
#define SAMPLE_FORMAT_LE_16BIT_SI   (1<<2)
#define SAMPLE_FORMAT_LE_24BIT_SI   (1<<3)
#define SAMPLE_FORMAT_LE_32BIT_SI   (1<<4)
#define SAMPLE_FORMAT_LE_32BIT_FLT  (1<<5)
#define SAMPLE_FORMAT_RAW_U8_DATA   (1<<6)        //used for firmware updates etc

typedef struct __attribute__( ( packed ) )
{
  uint32_t magic;
  uint8_t  protocol_version;
  int32_t  header_length; //includes 32-bit crc
  int16_t  data_len;      //data follows crc32, does not exceed 1024 bytes
  uint32_t freq_hz;
  float    bb_dec_gain;
  float    bb_stddev;
  uint32_t sample_rate_hz;
  uint8_t  sample_format;
  uint32_t sequence;  //bit 31 requests ack to sequence, SDR_COMMAND_ACK is used to ack a sequence
  uint8_t  command;
  int16_t  rssi_dbm;
  uint8_t  lqi;     //link quality 0-100  (somewhat standards based)
  uint8_t  debug_mode;
  uint8_t  command_args[32];
  uint32_t validate_crc32;
}
sdr_header_packet;

typedef struct __attribute__( ( packed ) )
{
  sdr_header_packet header;
  uint8_t data[1024];
}
sdr_data_frame;

#define PROTO(x) x
#include "protos/udp_port_proto.h"

#endif
