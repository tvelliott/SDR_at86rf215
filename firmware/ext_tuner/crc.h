

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


#ifndef __CRC_H__
#define __CRC_H__

#include <stdint.h>
/* crc.c */
unsigned short CRCCCITT(uint8_t *data, int16_t length, uint16_t seed, uint16_t final);
uint32_t crc32_range(uint8_t *ucBuffer, int32_t ulCount);
void crc_test(void);
int8_t check_crc16(uint8_t *buffer);
int8_t update_crc16(uint8_t *buffer);
void clear_crc(void);
uint16_t get_crc(void);
uint16_t do_crc16(uint8_t data);
void update_struct_crc(uint8_t *sp, int16_t size);
uint8_t check_struct_crc(uint8_t *sp, int16_t size);
extern uint32_t crc32_val;

#endif
