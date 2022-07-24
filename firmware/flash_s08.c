
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



#include "stm32h7xx_hal.h"

#include <string.h>
#include <stdio.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include "std_io.h"
#include "globals.h"
#include "flash_s08.h"

#define FLASH_READ 0x03
#define FLASH_HIGH_SPEED_READ 0x0b
#define FLASH_ERASE_4k 0x20
#define FLASH_ERASE_32k 0x52
#define FLASH_ERASE_64k 0xd8
#define FLASH_CHIP_ERASE 0x60
#define FLASH_BYTE_PGM 0x02
#define FLASH_AAI_PGM 0xad
#define FLASH_RDSR 0x05
#define FLASH_EWSR 0x50
#define FLASH_WRSR 0x01
#define FLASH_WREN 0x06
#define FLASH_WRDI 0x04
#define FLASH_RDID 0x90
#define FLASH_EBSY 0x70
#define FLASH_DBSY 0x80

#define FLASH_BUSY (1<<0)
#define FLASH_WEL (1<<1)
#define FLASH_BP0 (1<<2)
#define FLASH_BP1 (1<<3)
#define FLASH_BP2 (1<<4)
#define FLASH_BP3 (1<<5)
#define FLASH_AAI (1<<6)
#define FLASH_BPL (1<<7)

uint8_t manf_id[2];
static uint32_t flash_addr;
static uint8_t *flash_ptr;

volatile int flash0_busy_flg;
volatile int flash1_busy_flg;

static int flash_initialized = 0;

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
void init_flash_s08( void )
{
  GPIO_InitTypeDef GPIO_InitStruct;

  HAL_SPI_MspInit( &hspi3 );

  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_SPI3_CLK_ENABLE();

  GPIO_InitStruct.Pin = GPIO_PIN_7; //at86rf215 cs  (controlled by fpga pin)
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  HAL_GPIO_Init( GPIOC, &GPIO_InitStruct );


  GPIO_InitStruct.Pin = GPIO_PIN_8;   //flash cs
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  HAL_GPIO_Init( GPIOC, &GPIO_InitStruct );

  HAL_GPIO_WritePin( GPIOC, GPIO_PIN_8, GPIO_PIN_SET );


  flash_initialized = 1;

}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
static void flash_select( int id )
{
  //if(!flash_initialized) init_flash_s08();

  DelayClk3( 250 );
  HAL_GPIO_WritePin( GPIOC, GPIO_PIN_8, GPIO_PIN_RESET );
  DelayClk3( 250 );
  //HAL_Delay(1);
}
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
static void flash_deselect( int id )
{
  //if(!flash_initialized) init_flash_s08();

  DelayClk3( 250 );
  HAL_GPIO_WritePin( GPIOC, GPIO_PIN_8, GPIO_PIN_SET );
  DelayClk3( 250 );
  //HAL_Delay(1);
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
uint8_t xmit_spi3( uint8_t out_reg )
{
  uint8_t in_reg;
  uint8_t tx = out_reg;

  //HAL_StatusTypeDef status;
  //status = HAL_SPI_TransmitReceive(&hspi3, &tx, &in_reg, 1, 0xff); //1 byte transfer, timeout
  //status = HAL_SPI_Transmit(&hspi3, &tx, 1, 0x00); //1 byte transfer, timeout
  //status = HAL_SPI_Receive(&hspi3, &in_reg, 1, 0x00); //1 byte transfer, timeout
  DelayClk3( 250 );
  HAL_SPI_TransmitReceive_IT( &hspi3, &tx, &in_reg, 1 );
  DelayClk3( 250 );
  return in_reg;
}


//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
void wait_for_flash( int id )
{
  //uint32_t prim;
  //prim = __get_PRIMASK();
  //__disable_irq();

  uint32_t stime = HAL_GetTick();

  //if(flash_timeouts>128) {
  //DelayClk3(1000);
  //return;
  //}

  while( flash_isbusy( id ) ) {
    //main_tick();
    if( HAL_GetTick() - stime > 3000 ) {
      //printf("\r\nflash timeout");
      return;
    }
  }


  //if(!prim) {
  // __enable_irq();
  //}
}


//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
uint8_t *flash_get_manf_id( int id )
{
  //uint32_t prim;
  //prim = __get_PRIMASK();
  //__disable_irq();


  //select flash
  flash_select( id );

  xmit_spi3( FLASH_RDID );
  xmit_spi3( 0x00 );
  xmit_spi3( 0x00 );
  xmit_spi3( 0x00 );

  manf_id[0] = xmit_spi3( 0xff );

  //deselect flash
  flash_deselect( id );



  //select flash
  flash_select( id );

  xmit_spi3( FLASH_RDID );
  xmit_spi3( 0x00 );
  xmit_spi3( 0x00 );
  xmit_spi3( 0x01 );

  manf_id[1] = xmit_spi3( 0xff );

  //deselect flash
  flash_deselect( id );

  //if(!prim) {
  // __enable_irq();
  //}
  return manf_id;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
void flash_unprotect_all( int id )
{

  //uint32_t prim;
  //prim = __get_PRIMASK();
  //__disable_irq();


  //select flash
  flash_select( id );

  xmit_spi3( FLASH_EWSR ); //enable status register write

  //deselect flash
  flash_deselect( id );


  //select flash
  flash_select( id );

  xmit_spi3( FLASH_WRSR );
  xmit_spi3( 0x00 ); //disable write protect

  //deselect flash
  flash_deselect( id );

  //if(!prim) {
  // __enable_irq();
  //}
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
void flash_read( uint32_t address, uint8_t *buffer, int16_t len, int id )
{

  int16_t i;

  //uint32_t prim;
  //prim = __get_PRIMASK();
  //__disable_irq();

  wait_for_flash( id );

  //select flash
  flash_select( id );

  xmit_spi3( FLASH_READ );

  xmit_spi3( ( address >> 16 ) );
  xmit_spi3( ( address >> 8 ) );
  xmit_spi3( ( address & 0xff ) );

  for( i = 0; i < len; i++ ) {
    buffer[i] = xmit_spi3( 0xff );
  }


  //deselect flash
  flash_deselect( id );


  //if(!prim) {
  // __enable_irq();
  //}

}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
void test_flash( int id )
{
  int i;
  int j;

  uint8_t test_buffer[1024];
  uint8_t test_buffer2[1024];

  uint32_t randomizer = HAL_GetTick();


  if( id != 0 && id != 1 ) id = 0;

#if 0
  flash_get_manf_id( 0 );
  printf( "\r\n%02x%02x", manf_id[0], manf_id[1] );
  return;
#endif

  //printf("\r\nerasing 256Kx8 flash, id: %d", id);
  //flash_erase_all(id);

  uint32_t start_addr = 4; //skip first 4k (user configuration)

  for( i = start_addr; i < 256; i++ ) {
    printf( "\r\nwriting 1k block %d, address %08x", i, i * 1024 );

    srand( i + randomizer );
    for( j = 0; j < 1024; j++ ) {
      test_buffer[j] = rand() % 255;
    }

    flash_write_blocking( ( uint32_t ) i * 1024, test_buffer, 1024, id );
    main_tick();
  }

  int errors = 0;
  //verify from mcu to external flash
  for( i = start_addr; i < 256; i++ ) {

    memset( test_buffer2, 0x00, sizeof( test_buffer2 ) );

    srand( i + randomizer );
    for( j = 0; j < 1024; j++ ) {
      test_buffer[j] = rand() % 255;
    }

    flash_read( ( uint32_t ) i * 1024, test_buffer2, 1024, id );

    for( j = 0; j < 1024; j++ ) {
      if( j == 0 ) printf( "\r\nverifying block %d, address %08x sample val %02x", i, i * 1024, test_buffer2[j] );

      if( test_buffer[j] != test_buffer2[j] ) {
        printf( "\r\nproblem verifying flash: %02x, %02x", test_buffer[j], test_buffer2[j] );
        errors++;
        //return;
      }

      main_tick();
    }
  }

  if( errors == 0 ) printf( "\r\nverified okay." );
  else printf( "\r\nfinished with %d errors", errors );
}


//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
void flash_write_blocking( uint32_t address, uint8_t *buffer, int32_t len, int id )
{

  int16_t i;

  //uint32_t prim;
  //prim = __get_PRIMASK();
  //__disable_irq();

  wait_for_flash( id );

  flash_addr = address;
  flash_ptr = buffer;


  for( i = 0; i < len; i++ ) {
    //auto erase
    if( ( flash_addr & 0xfff ) == 0 ) {
      flash_erase_4k( flash_addr, id );
      //printf("\r\nerasing 4k %x", flash_addr);
    }

    flash_write_byte( flash_addr++, *flash_ptr++, id );
  }


  //if(!prim) {
  // __enable_irq();
  //}

}
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
void flash_write_byte( uint32_t address, uint8_t val, int id )
{


  //uint32_t prim;
  //prim = __get_PRIMASK();
  //__disable_irq();

  wait_for_flash( id );

  //select flash
  flash_select( id );

  xmit_spi3( FLASH_WREN );   //enable write / erase

  //deselect flash
  flash_deselect( id );

  //select flash
  flash_select( id );

  xmit_spi3( FLASH_BYTE_PGM );

  xmit_spi3( ( uint8_t )( address >> 16 ) & 0xff );
  xmit_spi3( ( uint8_t )( address >> 8 ) & 0xff );
  xmit_spi3( ( uint8_t )( address & 0xff ) );

  xmit_spi3( val );

  //deselect flash
  flash_deselect( id );

  //if(!prim) {
  // __enable_irq();
  //}


}
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
void flash_erase_4k( uint32_t erase_addr, int id )
{

  //uint32_t prim;
  //prim = __get_PRIMASK();
  //__disable_irq();


  wait_for_flash( id );

  flash_unprotect_all( id );

  //select flash
  flash_select( id );

  xmit_spi3( FLASH_WREN );   //enable write / erase

  //deselect flash
  flash_deselect( id );



  //select flash
  flash_select( id );

  xmit_spi3( FLASH_ERASE_4k ); //do erase op
  xmit_spi3( ( uint8_t )( erase_addr >> 16 ) & 0xff );
  xmit_spi3( ( uint8_t )( erase_addr >> 8 ) & 0xff );
  xmit_spi3( ( uint8_t )( erase_addr & 0xff ) );

  //deselect flash
  flash_deselect( id );

  //if(!prim) {
  // __enable_irq();
  //}
}
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
void flash_erase_32k( uint32_t erase_addr, int id )
{

  //uint32_t prim;
  //prim = __get_PRIMASK();
  //__disable_irq();


  wait_for_flash( id );

  flash_unprotect_all( id );

  //select flash
  flash_select( id );

  xmit_spi3( FLASH_WREN );   //enable write / erase

  //deselect flash
  flash_deselect( id );



  //select flash
  flash_select( id );

  xmit_spi3( FLASH_ERASE_32k );  //do erase op
  xmit_spi3( ( uint8_t )( erase_addr >> 16 ) & 0xff );
  xmit_spi3( ( uint8_t )( erase_addr >> 8 ) & 0xff );
  xmit_spi3( ( uint8_t )( erase_addr & 0xff ) );

  //deselect flash
  flash_deselect( id );

  //if(!prim) {
  // __enable_irq();
  //}

}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
void flash_erase_64k( uint32_t erase_addr, int id )
{

  //uint32_t prim;
  //prim = __get_PRIMASK();
  //__disable_irq();


  wait_for_flash( id );

  flash_unprotect_all( id );

  //select flash
  flash_select( id );

  xmit_spi3( FLASH_WREN );   //enable write / erase

  //deselect flash
  flash_deselect( id );



  //select flash
  flash_select( id );

  xmit_spi3( FLASH_ERASE_64k );  //do erase op
  xmit_spi3( ( uint8_t )( erase_addr >> 16 ) & 0xff );
  xmit_spi3( ( uint8_t )( erase_addr >> 8 ) & 0xff );
  xmit_spi3( ( uint8_t )( erase_addr & 0xff ) );

  //deselect flash
  flash_deselect( id );

  //if(!prim) {
  // __enable_irq();
  //}
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
void flash_erase_all( int id )
{

  flash_unprotect_all( id );


  //select flash
  flash_select( id );

  xmit_spi3( FLASH_WREN );   //enable write / erase

  //deselect flash
  flash_deselect( id );



  //select flash
  flash_select( id );

  xmit_spi3( FLASH_CHIP_ERASE ); //do erase op

  //deselect flash
  flash_deselect( id );

}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
uint8_t flash_isbusy( int id )
{

  uint8_t status;


  //uint32_t prim;
  //prim = __get_PRIMASK();
  //__disable_irq();


  //select flash
  flash_select( id );

  xmit_spi3( FLASH_RDSR );
  status = xmit_spi3( 0xff );

  //printf("\r\nflash status: %02x", status);

  //deselect flash
  flash_deselect( id );



  //if(!prim) {
  //__enable_irq();
  //}
  if( status & FLASH_BUSY ) return 1;
  else return 0;
}
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
uint8_t flash_isprotected( int id )
{

  uint8_t status;

  //uint32_t prim;
  //prim = __get_PRIMASK();
  //__disable_irq();


  //select flash
  flash_select( id );

  xmit_spi3( FLASH_RDSR );
  status = xmit_spi3( 0xff );

  //printf("\r\nflash status: %02x", status);

  //deselect flash
  flash_deselect( id );



//  if(!prim) {
//   __enable_irq();
  //}

  if( status & ( FLASH_BP0 | FLASH_BP1 | FLASH_BP2 | FLASH_BP3 ) ) return 1;
  else return 0;
}
