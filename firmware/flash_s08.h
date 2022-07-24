
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



#ifndef __FLASH_S08_H__
#define __FLASH_S08_H__

#include "stm32h7xx_hal.h"

uint8_t flash_isprotected( int id );
void flash_erase_64k( uint32_t erase_addr, int id );
void flash_erase_32k( uint32_t erase_addr, int id );
void flash_write_byte( uint32_t address, uint8_t val, int id );
void flash_erase_4k( uint32_t erase_addr, int id );
void flash_write_blocking( uint32_t address, uint8_t *buffer, int32_t len, int id );
void flash_erase_all( int id );
void test_flash( int id );
void flash_read( uint32_t address, uint8_t *buffer, int16_t len, int id );
void flash_unprotect_all( int id );
void init_flash_s08( void );
uint8_t *flash_get_manf_id( int id );
uint8_t flash_isbusy( int id );
uint8_t xmit_spi3( uint8_t tx );
void wait_for_flash( int id );
extern volatile int flash1_busy_flg;
extern volatile int flash0_busy_flg;
extern uint8_t manf_id[2];
#endif
