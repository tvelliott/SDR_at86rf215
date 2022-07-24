/**
  ******************************************************************************
  * @file    stm32h7xx_it.c
  * @brief   Interrupt Service Routines.
  ******************************************************************************
  *
  * COPYRIGHT(c) 2018 STMicroelectronics
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */
/* Includes ------------------------------------------------------------------*/
#include "stm32h7xx_hal.h"
#include "stm32h7xx.h"
#include "stm32h7xx_it.h"
#include "fpga.h"
#include "globals.h"

#include <string.h>

extern SPI_HandleTypeDef hspi3;
extern int do_gqrx;

static int16_t *ptr_iq1 = ( void * ) 0x60004000;
static int16_t *ptr_iq2 = ( void * ) 0x60006000;
static volatile int rf900_cnt;
volatile uint16_t iq_data[512];
volatile int8_t iq_data_ready;
//static volatile uint16_t *addrptr = iq_addr;

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/* External variables --------------------------------------------------------*/

/******************************************************************************/
/*            Cortex Processor Interruption and Exception Handlers         */
/******************************************************************************/

/**
* @brief This function handles System tick timer.
*/
void SysTick_Handler( void )
{
  /* USER CODE BEGIN SysTick_IRQn 0 */

  /* USER CODE END SysTick_IRQn 0 */
  HAL_IncTick();
  HAL_SYSTICK_IRQHandler();
  /* USER CODE BEGIN SysTick_IRQn 1 */

  //print_stackptr();

  /* USER CODE END SysTick_IRQn 1 */
}

//////////////////////////////////////////////////////////////////////////////////////////
// rf900 rx fifo int
//////////////////////////////////////////////////////////////////////////////////////////
void EXTI1_IRQHandler( void )
{
  //clear the flag
  __HAL_GPIO_EXTI_CLEAR_IT( GPIO_PIN_1 );

  int i;

  rx_fifo0_900_ready = HAL_GPIO_ReadPin( GPIOF, GPIO_PIN_1 ); //MSB of the buffer driven by FPGA

  if( rx_fifo0_900_ready ) {
    memcpy( ( int16_t * ) iq_data, ( int16_t * ) ptr_iq1, 512 );
    //for(i=0;i<256;i++) {
    // iq_data[i] = ptr_iq1[i];
    //}
  } else {
    memcpy( ( int16_t * ) iq_data, ( int16_t * ) ptr_iq2, 512 );
    //for(i=0;i<256;i++) {
    // iq_data[i] = ptr_iq2[i];
    //}
  }

#if 0
  //do_handle_exti1=1;
  process_incoming_rf();
  //udp_send_response_iq900();  //this can't really wait much
#elseif 0
  //do_handle_exti1=1;
  udp_send_response_iq900();  //this can't really wait much
#endif

  if( do_gqrx ) {
#if 0
    //iq_data_ready=1;  //for TCP
    //rtl_tcp_send();
#else
    udp_send_response_iq900( ( uint8_t * ) iq_data, 512 ); //for UDP
#endif

  } else {
    process_incoming_rf();
  }

}

//////////////////////////////////////////////////////////////////////////////////////////
// audio fill fifo int
//////////////////////////////////////////////////////////////////////////////////////////
void EXTI0_IRQHandler( void )
{
  //clear the flag
  __HAL_GPIO_EXTI_CLEAR_IT( GPIO_PIN_0 );


  //figure out which half of the buffer to fill
  low_side = HAL_GPIO_ReadPin( GPIOF, GPIO_PIN_0 ); //MSB of the buffer driven by FPGA
  if( low_side ) {
    audio_fifo_addr_offset = 0x0000;
  } else {
    audio_fifo_addr_offset = AUDIO_BUFFER_SIZE / 2;
  }

  do_audio_fifo_fill();

}

//////////////////////////////////////////////////////////////////////////////////////////
//  not used, but interrupt version of spi transfer is only function that works in HAL
//////////////////////////////////////////////////////////////////////////////////////////
void SPI3_IRQHandler( void )
{
  HAL_SPI_IRQHandler( &hspi3 );
}
