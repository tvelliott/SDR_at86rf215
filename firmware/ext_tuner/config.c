

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


#include <stdint.h>
#include <string.h>
#include "crc.h"
#include "global.h"
#include "config.h"
#include "stm32f4xx_flash.h"

sys_config config;
uint32_t *pulDeviceType;
int32_t hardware_revision;

//////////////////////////////////////////////////
//////////////////////////////////////////////////
void read_config()
{
  static int8_t *ptr = (int8_t *) FLASH_USER_START_ADDR;

  //validate configuration or revert to defaults
  if ( check_struct_crc( (uint8_t *) FLASH_USER_START_ADDR, sizeof(sys_config)) ) {
    reset_config_to_defaults();
    write_config(&config);
  } else {
    memcpy(&config, ptr, sizeof(sys_config) );
  }

  //pulDeviceType = (unsigned long *) 0xf0; //pull the hardware rev out of the bootloader vector table
  //hardware_revision = pulDeviceType[0];

  config_changed();
}
//////////////////////////////////////////////////
//////////////////////////////////////////////////
void config_changed()
{
  //if (config.mode == ) {
  //}
}

////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////
static uint32_t GetSector(uint32_t Address)
{
  uint32_t sector = 0;

  if((Address < ADDR_FLASH_SECTOR_1) && (Address >= ADDR_FLASH_SECTOR_0)) {
    sector = FLASH_Sector_0;
  } else if((Address < ADDR_FLASH_SECTOR_2) && (Address >= ADDR_FLASH_SECTOR_1)) {
    sector = FLASH_Sector_1;
  } else if((Address < ADDR_FLASH_SECTOR_3) && (Address >= ADDR_FLASH_SECTOR_2)) {
    sector = FLASH_Sector_2;
  } else if((Address < ADDR_FLASH_SECTOR_4) && (Address >= ADDR_FLASH_SECTOR_3)) {
    sector = FLASH_Sector_3;
  } else if((Address < ADDR_FLASH_SECTOR_5) && (Address >= ADDR_FLASH_SECTOR_4)) {
    sector = FLASH_Sector_4;
  } else if((Address < ADDR_FLASH_SECTOR_6) && (Address >= ADDR_FLASH_SECTOR_5)) {
    sector = FLASH_Sector_5;
  } else if((Address < ADDR_FLASH_SECTOR_7) && (Address >= ADDR_FLASH_SECTOR_6)) {
    sector = FLASH_Sector_6;
  } else if((Address < ADDR_FLASH_SECTOR_8) && (Address >= ADDR_FLASH_SECTOR_7)) {
    sector = FLASH_Sector_7;
  } else if((Address < ADDR_FLASH_SECTOR_9) && (Address >= ADDR_FLASH_SECTOR_8)) {
    sector = FLASH_Sector_8;
  } else if((Address < ADDR_FLASH_SECTOR_10) && (Address >= ADDR_FLASH_SECTOR_9)) {
    sector = FLASH_Sector_9;
  } else if((Address < ADDR_FLASH_SECTOR_11) && (Address >= ADDR_FLASH_SECTOR_10)) {
    sector = FLASH_Sector_10;
  } else { /*(Address < FLASH_END_ADDR) && (Address >= ADDR_FLASH_SECTOR_11))*/
    sector = FLASH_Sector_11;
  }

  return sector;
}


//////////////////////////////////////////////////
//////////////////////////////////////////////////
void write_config( sys_config *configtmp)
{
  uint32_t StartSector = 0, EndSector = 0, Address = 0, i = 0 ;
  __IO uint32_t data32 = 0 , MemoryProgramStatus = 0 ;

  uint32_t GetSector(uint32_t Address);

  //validate configuration
  update_struct_crc( (uint8_t *) configtmp, sizeof(sys_config) );

  FLASH_Unlock();

  // Clear pending flags (if any)
  FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR |
                  FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR|FLASH_FLAG_PGSERR);

  StartSector = GetSector(FLASH_USER_START_ADDR);
  //not used
  EndSector = GetSector(FLASH_USER_END_ADDR);

  //erase sector
  if (FLASH_EraseSector(StartSector, VoltageRange_3) != FLASH_COMPLETE) {
    //error if we get here
  }

  int config_size_w32 = (sizeof(sys_config)/4)+1;
  uint32_t *ptr = (uint32_t *) configtmp;

  Address = FLASH_USER_START_ADDR;
  //write configuration to flash in 32 bit words
  for(i=0; i<config_size_w32; i++) {
    if (FLASH_ProgramWord(Address, *ptr++) != FLASH_COMPLETE) {
      //error if we get here
    } else {
      Address+=4;
    }
  }

  FLASH_Lock();

  //TODO: verify configuration
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
//volatile uint8_t  unit_addr;    //addr
//volatile uint8_t  remote_addr;  //setcon
//volatile uint8_t  network;
//volatile double   freqency;
//volatile int      frack;
//volatile int      packlength;
//volatile uint16_t  config_crc;  //don't remove
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
void reset_config_to_defaults(void)
{
  config.unit_addr = 80;
  config.remote_addr = 5;
  config.network = 37;
  config.frequency = 455.0000f;
  config.packlength = 2000;

  config_changed();
}
