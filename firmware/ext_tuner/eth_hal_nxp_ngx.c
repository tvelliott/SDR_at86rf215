

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


#include "global.h"
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include "crc.h"
#include "eth_hal_nxp_ngx.h"



#define ETH_FRAME_SIZE 1536
//#define DEBUG_EEM 1

//can we get rid of one or more buffers?
volatile uint8_t eth_frame_buffer[ ETH_FRAME_SIZE ];
volatile uint8_t ethernet_buffer[ ETH_FRAME_SIZE ]; //ethernet frame size
volatile uint8_t ethernet_out_buffer[ ETH_FRAME_SIZE ];

uint8_t eem_sentinal_deadbeef[]= {0xde,0xad,0xbe,0xef};
volatile uint8_t *frame_ptr;
volatile int32_t frame_len;
enum {
  ETH_STATE_SOF,
  ETH_STATE_RX,
  ETH_STATE_VALIDATION,
};
volatile int16_t  eth_state = ETH_STATE_SOF;
extern volatile int16_t eem_nbytes;


extern volatile int16_t eth_state;

#define EEM_FRAME_TYPE_INVALID 0
#define EEM_FRAME_TYPE_CMD 1
#define EEM_FRAME_TYPE_DATA 2
volatile uint8_t eem_frame_type;

volatile int16_t ethernet_frame_len;
volatile int16_t eem_nbytes;
volatile int32_t eem_rxcount;
volatile int32_t eem_txcount;

extern void *pdev_device;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// internal function
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static void eem_incoming_cbf(uint8_t *eth_frame_buffer, int16_t len)
{
#ifdef DEBUG_EEM
  printf("\r\neem_rx %d", len);
#endif

  //drop incoming if current hasn't been processed
  if(ethernet_frame_len==0) {
    memcpy( (char *) ethernet_buffer, (char *) eth_frame_buffer, len);
    ethernet_frame_len = len;
    eem_rxcount++;

  }
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
void eem_init_state()
{
  frame_ptr = eth_frame_buffer;
  eth_state = ETH_STATE_SOF;
  eem_nbytes=0;

}

/////////////////////////////////////////////////////////////////////////////////////
//  this should be called everytime an eem bulkout endpoint receives data frame or nak
//  usually called from USB_RX interrupt
/////////////////////////////////////////////////////////////////////////////////////
void EEM_BulkOut(uint8_t *eem_in_buffer, int32_t eem_nbytes)
{
#ifdef DEBUG_EEM
  printf("\r\neem_in %d", eem_nbytes);
#endif

  //check for overrun
  if( ((uint32_t) eem_nbytes + frame_ptr) > &eth_frame_buffer[ ETH_FRAME_SIZE-1 ] ) {
    eth_state = ETH_STATE_SOF;
    frame_ptr = eth_frame_buffer;
    eem_nbytes=0;
#ifdef DEBUG_EEM
    printf("\r\neem: overrun");
#endif
    return;
  }

  switch( eth_state ) {

  case  ETH_STATE_SOF :
eth_sof:
    //initialize these first in case of goto eth_sof
    eth_state = ETH_STATE_SOF;
    frame_ptr = eth_frame_buffer;

    //is this a cmd or data??  if data, then do some extra checks
    if( ! (eth_frame_buffer[1]&0x80) ) {

      //must not be ethernet frame
      if(eem_nbytes < 24 ) return;

      //make sure looks like ethernet frame type
      if( eem_in_buffer[12+2]==0x08 && (eem_in_buffer[13+2]==0x06 || eem_in_buffer[13+2]==0x00) ) {
        eth_state = ETH_STATE_RX;
        frame_ptr = eth_frame_buffer;
        goto eth_rx;
      }
    } else if( ! (eth_frame_buffer[1]&0xc0) ) { //bit 14 is reserved and should never be set for cmd packet
      eth_state = ETH_STATE_RX;
      frame_ptr = eth_frame_buffer;
      goto eth_rx;
    }

    return; //bad header

    break;

  case  ETH_STATE_RX  :

eth_rx:

    //first segment? check the header bit15 to see if this is a command frame
    if( frame_ptr == eth_frame_buffer && (eth_frame_buffer[1]&0x80)) {

      //bit 14 must be zero (reserved)
      if( (eth_frame_buffer[1]&0x40)==0) {
        eem_frame_type = EEM_FRAME_TYPE_CMD;


        //mask off cmd bits 13-11
        switch(  (eth_frame_buffer[1] & 0x38)>>3 ) {

          //echo  (mandatory response as per specs), Windows drivers might need it.  Linux doesn't currently.
        case  0 :

          //linux CDC EEM host-side driver doesn't send echo cmd
#ifdef DEBUG_EEM
          printf("\r\neem: echo request %02x%02x", eth_frame_buffer[1], eth_frame_buffer[0]);
#endif

          //TODO: need to set flag and store length for echo request and send an echo response
          //which may be larger than >64 bytes
          //bits 10-0 are the echo data length
          break;

          ////////////////////////////////////////////////////////////////////////////////
          //the rest of the commands appear to be optional from looking at the specs
          ////////////////////////////////////////////////////////////////////////////////

          //linux does respond to echo requests as per specs, so we can test this
          //echo response
        case  1 :

          //suspend hint
        case  2 :

          //response hint
        case  3 :

          //response complete hint
        case  4 :

          //tickle
        case  5 :

          break;

        default   :
          //NOT AN EEM HEADER
          eth_state = ETH_STATE_SOF;
          frame_ptr = eth_frame_buffer;
          eem_nbytes=0;
#ifdef DEBUG_EEM
          printf("\r\neem: bad header");
#endif
          return;

        }
      } else {
        //NOT AN EEM HEADER
        eth_state = ETH_STATE_SOF;
        frame_ptr = eth_frame_buffer;
        eem_nbytes=0;
        eem_frame_type = EEM_FRAME_TYPE_INVALID;
#ifdef DEBUG_EEM
        printf("\r\neem: bad header");
#endif
        return;
      }
    } else if( frame_ptr == eth_frame_buffer && !(eth_frame_buffer[1]&0x80)) {
      eem_frame_type = EEM_FRAME_TYPE_DATA;
    }



    //add new segment
    if( eem_nbytes == 64) {
      memcpy( (char *) frame_ptr, (char *) eem_in_buffer, eem_nbytes);
      frame_ptr+=eem_nbytes;

    } else if( eem_nbytes < 64) {

      if(eem_frame_type == EEM_FRAME_TYPE_DATA) {

        //validate frame here

        //get the frame length in EEM header
        uint16_t *sptr = eth_frame_buffer;
        //this is first seg?
        if(frame_ptr == eth_frame_buffer) {
          sptr = eem_in_buffer;
        }
        int16_t hlen = *sptr;
        //mask off bits 14,15
        hlen &= 0x3fff;
        //sub frame crc length
        hlen -= 4;

        frame_len = ( (frame_ptr+eem_nbytes) - eth_frame_buffer)-6;

        //bad frame length?
        if(frame_len != hlen) {
#ifdef DEBUG_EEM
          printf("\r\neem: frame_len != hlen");
#endif
          //goto eth_sof;
          if(eem_nbytes==2) {
            eem_nbytes=0;
            frame_len-=2;
          }
        }

        //copy in last segment
        if(eem_nbytes>0) {
          memcpy( (char *) frame_ptr, (char *) eem_in_buffer, eem_nbytes);
          frame_ptr+=eem_nbytes;
        }

        if(frame_len>0 && frame_len<ETH_FRAME_SIZE) {

          //validate frame crc
          crc32_val = ~0L;
          uint32_t eth_crc = crc32_range( &eth_frame_buffer[2], frame_len );
          eth_crc = ~eth_crc;

          int8_t skipcrc=0;

          if( (eth_frame_buffer[1]&0x40)==0x00 && memcmp(&eth_frame_buffer[2+frame_len], eem_sentinal_deadbeef, 4)==0) {
            skipcrc=1;
          }

          if( skipcrc || memcmp( &eth_crc, &eth_frame_buffer[2+frame_len], 4)==0) {

            eth_state = ETH_STATE_SOF;
            frame_ptr = eth_frame_buffer;

            //we got a good Ethernet frame from host , so deliver it to next layer up via call-back function
            eem_incoming_cbf( &eth_frame_buffer[2], frame_len);

            return;
          } else {

            //bad crc
            //eth_state = ETH_STATE_SOF;
            //frame_ptr = eth_frame_buffer;
            //eem_nbytes=0;
#ifdef DEBUG_EEM
            printf("\r\neem: bad frame crc");
#endif
            //return;

            goto eth_sof;
          }
        } else {

          //eth_state = ETH_STATE_SOF;
          //frame_ptr = eth_frame_buffer;
#ifdef DEBUG_EEM
          printf("\r\neem: bad frame length: %d", eem_nbytes);
#endif
          //eem_nbytes=0;
          //return;

          goto eth_sof;
        }
      } //eem frame type DATA
      else if(eem_frame_type == EEM_FRAME_TYPE_CMD) {
#ifdef DEBUG_EEM
        printf("\r\ncmd frame");
#endif
        //TODO: handle cmd frame
        goto eth_sof;
      } //eem frame type CMD
      else {
#ifdef DEBUG_EEM
        printf("\r\neem error unknown");
#endif
        goto eth_sof;
      } //invalid frame type


    } //nbytes < 64

    eem_nbytes=0;
    break;

  case  ETH_STATE_VALIDATION  :
  default   :
    eth_state = ETH_STATE_SOF;
    frame_ptr = eth_frame_buffer;
    eem_nbytes=0;
    break;
  }

}

/////////////////////////////////////////////////////////////////////////////////
//  send frame using EEM  (Ethernet over usb)
//  max throuput in a tight loop @168Mhz w/max eth frame size is:
/////////////////////////////////////////////////////////////////////////////////
void eem_send_frame(uint8_t *frameptr, int16_t len)
{

  //if( (len+6) % 64 ==0) len++;  //don't allow 64 multiples, add padding

  eem_txcount++;
  //memset(ethernet_out_buffer,0x00,64);

  //make room for 16-bit EEM header at beginning
  memcpy( (char *) &ethernet_out_buffer[2], (char *) frameptr, len);

  //calculate frame crc
  crc32_val = ~0L;
  uint32_t eth_crc = crc32_range( &ethernet_out_buffer[2], len );
  eth_crc = ~eth_crc;

  //move frame crc to end of frame
  memcpy( (char *) &ethernet_out_buffer[len+2], (char *) &eth_crc, 4);

  //add frame_crc to header len field
  uint16_t eem_header = len+4;
  eem_header |= 0x4000;  //set check frame crc bit, since we don't send the 0xdeadbeef sentinal frame crc

  //move eem header to beg of frame
  memcpy( (char *) ethernet_out_buffer, (char *) &eem_header, 2);

  //add 6 to len for eem header and frame crc
  int16_t send_len = len+6;


  //USB_WriteEP (ethernet_out_buffer, send_len);
  //DelayClk3( 8400 * 5); //based on 168MHz system clock
  //USB_OTG_BSP_uDelay(15*send_len);

#define WEPDEL 100

  //need to send multiple segments?
  if(send_len > 64) {

    int16_t offset=0;
    while( send_len > 0) {
      if(send_len > 64) {
        USB_WriteEP (&ethernet_out_buffer[offset], 64);
        DelayClk3( 8400 /WEPDEL); //based on 168MHz system clock

        send_len-=64;
        offset+=64;
      } else {
        //last segment
        USB_WriteEP (&ethernet_out_buffer[offset], send_len);
        DelayClk3( 8400 /WEPDEL); //based on 168MHz system clock
        if(send_len==64) {
          //is this to send ZLP?
          USB_WriteEP (&ethernet_out_buffer[0], 0);
          //DCD_EP_Tx (pdev_device, 0, NULL, 1);
          DelayClk3( 8400 /WEPDEL); //based on 168MHz system clock
          //printf("\r\nsend zlp");
        }
        send_len = 0;
      }
    }
  } else {
    //send single frame segment
    USB_WriteEP (ethernet_out_buffer, send_len);
    DelayClk3( 8400 /WEPDEL); //based on 168MHz system clock

    if(send_len==64) {
      //is this to send ZLP?
      USB_WriteEP (&ethernet_out_buffer[0], 0);
      //DCD_EP_Tx (pdev_device, 0, NULL, 1);
      DelayClk3( 8400 /WEPDEL); //based on 168MHz system clock
      //printf("\r\nsend zlp");
    }
  }


}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  poll this function to receive EEM ethernet frames
//  returns 0 - noframe avail,  or >0  ethernet frame length
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int16_t eem_read_frame(uint8_t *buffer)
{
  if(ethernet_frame_len>0) {
    memcpy( (char *) buffer, (char *) ethernet_buffer, ethernet_frame_len);
    int16_t len = ethernet_frame_len;
    ethernet_frame_len = 0;
    return len;
  }
  return 0;
}
