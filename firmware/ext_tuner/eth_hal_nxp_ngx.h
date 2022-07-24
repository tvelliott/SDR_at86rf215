

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


#ifndef __EEM_HAL__
#define __EEM_HAL__
//extern volatile uint8_t ethernet_buffer[ 1536 ];
extern volatile uint8_t ethernet_out_buffer[ 1536 ];
extern volatile int16_t eth_state;

extern volatile int16_t ethernet_frame_len;
extern volatile int32_t eem_rxcount;
extern volatile int32_t eem_txcount;
/* eth_hal_nxp_ngx.c */
void eem_init_state(void);
void EEM_BulkOut(uint8_t *eem_in_buffer, int32_t eem_nbytes);
void USB_WriteEP(uint8_t *eem_in_buffer, int32_t eem_nbytes);
void eem_send_frame(uint8_t *frameptr, int16_t len);
int16_t eem_read_frame(uint8_t *buffer);
#endif
