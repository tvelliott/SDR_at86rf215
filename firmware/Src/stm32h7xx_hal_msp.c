/**
  ******************************************************************************
  * File Name          : stm32h7xx_hal_msp.c
  * Description        : This file provides code for the MSP Initialization
  *                      and de-Initialization codes.
  ******************************************************************************
  * This notice applies to any and all portions of this file
  * that are not between comment pairs USER CODE BEGIN and
  * USER CODE END. Other portions of this file, whether
  * inserted by the user or by software development tools
  * are owned by their respective copyright owners.
  *
  * Copyright (c) 2018 STMicroelectronics International N.V.
  * All rights reserved.
  *
  * Redistribution and use in source and binary forms, with or without
  * modification, are permitted, provided that the following conditions are met:
  *
  * 1. Redistribution of source code must retain the above copyright notice,
  *    this list of conditions and the following disclaimer.
  * 2. Redistributions in binary form must reproduce the above copyright notice,
  *    this list of conditions and the following disclaimer in the documentation
  *    and/or other materials provided with the distribution.
  * 3. Neither the name of STMicroelectronics nor the names of other
  *    contributors to this software may be used to endorse or promote products
  *    derived from this software without specific written permission.
  * 4. This software, including modifications and/or derivative works of this
  *    software, must execute solely and exclusively on microcontroller or
  *    microprocessor devices manufactured by or for STMicroelectronics.
  * 5. Redistribution and use of this software other than as permitted under
  *    this license is void and will automatically terminate your rights under
  *    this license.
  *
  * THIS SOFTWARE IS PROVIDED BY STMICROELECTRONICS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS, IMPLIED OR STATUTORY WARRANTIES, INCLUDING, BUT NOT
  * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
  * PARTICULAR PURPOSE AND NON-INFRINGEMENT OF THIRD PARTY INTELLECTUAL PROPERTY
  * RIGHTS ARE DISCLAIMED TO THE FULLEST EXTENT PERMITTED BY LAW. IN NO EVENT
  * SHALL STMICROELECTRONICS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
  * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
  * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
  * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
  * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
  * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
  * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */
/* Includes ------------------------------------------------------------------*/
#include "stm32h7xx_hal.h"
extern void _Error_Handler( char *, int );
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */
/**
  * Initializes the Global MSP.
  */
void HAL_MspInit( void )
{
  /* USER CODE BEGIN MspInit 0 */

  /* USER CODE END MspInit 0 */
  PWR_PVDTypeDef sConfigPVD;

  __HAL_RCC_SYSCFG_CLK_ENABLE();

  HAL_NVIC_SetPriorityGrouping( NVIC_PRIORITYGROUP_4 );

  /* System interrupt init*/
  /* MemoryManagement_IRQn interrupt configuration */
  HAL_NVIC_SetPriority( MemoryManagement_IRQn, 0, 0 );
  /* BusFault_IRQn interrupt configuration */
  HAL_NVIC_SetPriority( BusFault_IRQn, 0, 0 );
  /* UsageFault_IRQn interrupt configuration */
  HAL_NVIC_SetPriority( UsageFault_IRQn, 0, 0 );
  /* SVCall_IRQn interrupt configuration */
  HAL_NVIC_SetPriority( SVCall_IRQn, 0, 0 );
  /* DebugMonitor_IRQn interrupt configuration */
  HAL_NVIC_SetPriority( DebugMonitor_IRQn, 0, 0 );
  /* PendSV_IRQn interrupt configuration */
  HAL_NVIC_SetPriority( PendSV_IRQn, 0, 0 );
  /* SysTick_IRQn interrupt configuration */
  HAL_NVIC_SetPriority( SysTick_IRQn, 0, 0 );

  /**PVD Configuration
  */
  sConfigPVD.PVDLevel = PWR_PVDLEVEL_0;
  sConfigPVD.Mode = PWR_PVD_MODE_NORMAL;
  HAL_PWR_ConfigPVD( &sConfigPVD );

  /**Enable the PVD Output
  */
  HAL_PWR_EnablePVD();

  /**Configure the internal voltage reference buffer voltage scale
  */
  HAL_SYSCFG_VREFBUF_VoltageScalingConfig( SYSCFG_VREFBUF_VOLTAGE_SCALE0 );

  /**Enable the Internal Voltage Reference buffer
  */
  HAL_SYSCFG_EnableVREFBUF();

  /**Configure the internal voltage reference buffer high impedance mode
  */
  HAL_SYSCFG_VREFBUF_HighImpedanceConfig( SYSCFG_VREFBUF_HIGH_IMPEDANCE_DISABLE );

  /* USER CODE BEGIN MspInit 1 */

  /* USER CODE END MspInit 1 */
}

void HAL_ADC_MspInit( ADC_HandleTypeDef *hadc )
{

  GPIO_InitTypeDef GPIO_InitStruct;
  if( hadc->Instance == ADC1 ) {
    /* USER CODE BEGIN ADC1_MspInit 0 */

    /* USER CODE END ADC1_MspInit 0 */
    /* Peripheral clock enable */
    __HAL_RCC_ADC12_CLK_ENABLE();

    /**ADC1 GPIO Configuration
    PA4     ------> ADC1_INP18
    PA5     ------> ADC1_INP19
    */
    GPIO_InitStruct.Pin = GPIO_PIN_4 | GPIO_PIN_5;
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init( GPIOA, &GPIO_InitStruct );

    /* USER CODE BEGIN ADC1_MspInit 1 */

    /* USER CODE END ADC1_MspInit 1 */
  } else if( hadc->Instance == ADC3 ) {
    /* USER CODE BEGIN ADC3_MspInit 0 */

    /* USER CODE END ADC3_MspInit 0 */
    /* Peripheral clock enable */
    __HAL_RCC_ADC3_CLK_ENABLE();

    /**ADC3 GPIO Configuration
    PC2_C     ------> ADC3_INN1
    PC2_C     ------> ADC3_INP0
    PC3_C     ------> ADC3_INP1
    */
    HAL_SYSCFG_AnalogSwitchConfig( SYSCFG_SWITCH_PC2, SYSCFG_SWITCH_PC2_OPEN );

    HAL_SYSCFG_AnalogSwitchConfig( SYSCFG_SWITCH_PC3, SYSCFG_SWITCH_PC3_OPEN );

    /* USER CODE BEGIN ADC3_MspInit 1 */

    /* USER CODE END ADC3_MspInit 1 */
  }

}

void HAL_ADC_MspDeInit( ADC_HandleTypeDef *hadc )
{

  if( hadc->Instance == ADC1 ) {
    /* USER CODE BEGIN ADC1_MspDeInit 0 */

    /* USER CODE END ADC1_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_ADC12_CLK_DISABLE();

    /**ADC1 GPIO Configuration
    PA4     ------> ADC1_INP18
    PA5     ------> ADC1_INP19
    */
    HAL_GPIO_DeInit( GPIOA, GPIO_PIN_4 | GPIO_PIN_5 );

    /* USER CODE BEGIN ADC1_MspDeInit 1 */

    /* USER CODE END ADC1_MspDeInit 1 */
  } else if( hadc->Instance == ADC3 ) {
    /* USER CODE BEGIN ADC3_MspDeInit 0 */

    /* USER CODE END ADC3_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_ADC3_CLK_DISABLE();

    /* USER CODE BEGIN ADC3_MspDeInit 1 */

    /* USER CODE END ADC3_MspDeInit 1 */
  }

}

void HAL_SD_MspInit( SD_HandleTypeDef *hsd )
{

  GPIO_InitTypeDef GPIO_InitStruct;
  if( hsd->Instance == SDMMC2 ) {
    /* USER CODE BEGIN SDMMC2_MspInit 0 */

    /* USER CODE END SDMMC2_MspInit 0 */
    /* Peripheral clock enable */
    __HAL_RCC_SDMMC2_CLK_ENABLE();

    /**SDMMC2 GPIO Configuration
    PA0-WKUP     ------> SDMMC2_CMD
    PB14     ------> SDMMC2_D0
    PD6     ------> SDMMC2_CK
    */
    GPIO_InitStruct.Pin = GPIO_PIN_0;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF9_SDIO2;
    HAL_GPIO_Init( GPIOA, &GPIO_InitStruct );

    GPIO_InitStruct.Pin = GPIO_PIN_14;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF9_SDIO2;
    HAL_GPIO_Init( GPIOB, &GPIO_InitStruct );

    GPIO_InitStruct.Pin = GPIO_PIN_6;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF11_SDIO2;
    HAL_GPIO_Init( GPIOD, &GPIO_InitStruct );

    /* USER CODE BEGIN SDMMC2_MspInit 1 */

    /* USER CODE END SDMMC2_MspInit 1 */
  }

}

void HAL_SD_MspDeInit( SD_HandleTypeDef *hsd )
{

  if( hsd->Instance == SDMMC2 ) {
    /* USER CODE BEGIN SDMMC2_MspDeInit 0 */

    /* USER CODE END SDMMC2_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_SDMMC2_CLK_DISABLE();

    /**SDMMC2 GPIO Configuration
    PA0-WKUP     ------> SDMMC2_CMD
    PB14     ------> SDMMC2_D0
    PD6     ------> SDMMC2_CK
    */
    HAL_GPIO_DeInit( GPIOA, GPIO_PIN_0 );

    HAL_GPIO_DeInit( GPIOB, GPIO_PIN_14 );

    HAL_GPIO_DeInit( GPIOD, GPIO_PIN_6 );

    /* USER CODE BEGIN SDMMC2_MspDeInit 1 */

    /* USER CODE END SDMMC2_MspDeInit 1 */
  }

}

void HAL_SPI_MspInit( SPI_HandleTypeDef *hspi )
{

  GPIO_InitTypeDef GPIO_InitStruct;
  if( hspi->Instance == SPI3 ) {
    /* USER CODE BEGIN SPI3_MspInit 0 */

    /* USER CODE END SPI3_MspInit 0 */
    /* Peripheral clock enable */
    __HAL_RCC_SPI3_CLK_ENABLE();

    /**SPI3 GPIO Configuration
    PC10     ------> SPI3_SCK
    PC11     ------> SPI3_MISO
    PC12     ------> SPI3_MOSI
    */
    GPIO_InitStruct.Pin = GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF6_SPI3;
    HAL_GPIO_Init( GPIOC, &GPIO_InitStruct );

    HAL_NVIC_SetPriority( SPI3_IRQn, 3, 3 );
    HAL_NVIC_EnableIRQ( SPI3_IRQn );


    /* USER CODE BEGIN SPI3_MspInit 1 */
    //printf("\r\ninitialized SPI3 I/O");

    /* USER CODE END SPI3_MspInit 1 */
  }

}

void HAL_SPI_MspDeInit( SPI_HandleTypeDef *hspi )
{

  if( hspi->Instance == SPI3 ) {
    /* USER CODE BEGIN SPI3_MspDeInit 0 */

    /* USER CODE END SPI3_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_SPI3_CLK_DISABLE();

    /**SPI3 GPIO Configuration
    PC10     ------> SPI3_SCK
    PC11     ------> SPI3_MISO
    PC12     ------> SPI3_MOSI
    */
    HAL_GPIO_DeInit( GPIOC, GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12 );

    /* USER CODE BEGIN SPI3_MspDeInit 1 */

    /* USER CODE END SPI3_MspDeInit 1 */
  }

}

void HAL_UART_MspInit( UART_HandleTypeDef *huart )
{

  GPIO_InitTypeDef GPIO_InitStruct;
  if( huart->Instance == USART3 ) {
    /* USER CODE BEGIN USART3_MspInit 0 */

    /* USER CODE END USART3_MspInit 0 */
    /* Peripheral clock enable */
    __HAL_RCC_USART3_CLK_ENABLE();

    /**USART3 GPIO Configuration
    PB10     ------> USART3_TX
    PB11     ------> USART3_RX
    */
    GPIO_InitStruct.Pin = GPIO_PIN_10 | GPIO_PIN_11;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF7_USART3;
    HAL_GPIO_Init( GPIOB, &GPIO_InitStruct );

    /* USER CODE BEGIN USART3_MspInit 1 */

    /* USER CODE END USART3_MspInit 1 */
  }

}

void HAL_UART_MspDeInit( UART_HandleTypeDef *huart )
{

  if( huart->Instance == USART3 ) {
    /* USER CODE BEGIN USART3_MspDeInit 0 */

    /* USER CODE END USART3_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_USART3_CLK_DISABLE();

    /**USART3 GPIO Configuration
    PB10     ------> USART3_TX
    PB11     ------> USART3_RX
    */
    HAL_GPIO_DeInit( GPIOB, GPIO_PIN_10 | GPIO_PIN_11 );

    /* USER CODE BEGIN USART3_MspDeInit 1 */

    /* USER CODE END USART3_MspDeInit 1 */
  }

}


static uint32_t FMC_Initialized = 0;

static void HAL_FMC_MspInit( void )
{
  /* USER CODE BEGIN FMC_MspInit 0 */

  /* USER CODE END FMC_MspInit 0 */
  GPIO_InitTypeDef GPIO_InitStruct;
  if( FMC_Initialized ) {
    return;
  }
  FMC_Initialized = 1;
  /* Peripheral clock enable */
  __HAL_RCC_FMC_CLK_ENABLE();

  /** FMC GPIO Configuration
  PG0   ------> FMC_A10
  PG1   ------> FMC_A11
  PE7   ------> FMC_D4
  PE8   ------> FMC_D5
  PE9   ------> FMC_D6
  PE10   ------> FMC_D7
  PE11   ------> FMC_D8
  PE12   ------> FMC_D9
  PE13   ------> FMC_D10
  PE14   ------> FMC_D11
  PE15   ------> FMC_D12
  PD8   ------> FMC_D13
  PD9   ------> FMC_D14
  PD10   ------> FMC_D15
  PD14   ------> FMC_D0
  PD15   ------> FMC_D1
  PG2   ------> FMC_A12
  PG3   ------> FMC_A13
  PG4   ------> FMC_A14
  PG5   ------> FMC_A15
  PD0   ------> FMC_D2
  PD1   ------> FMC_D3
  PD3   ------> FMC_CLK
  PD4   ------> FMC_NOE
  PD5   ------> FMC_NWE
  PD7   ------> FMC_NE1
  PB7   ------> FMC_NL
  */

  //GPIO_InitStruct.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5;
  //GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  //GPIO_InitStruct.Pull = GPIO_NOPULL;
  //GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  //GPIO_InitStruct.Alternate = GPIO_AF12_FMC;
  //HAL_GPIO_Init( GPIOG, &GPIO_InitStruct );

  GPIO_InitStruct.Pin = GPIO_PIN_7 | GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10
                        | GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14
                        | GPIO_PIN_15;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF12_FMC;
  HAL_GPIO_Init( GPIOE, &GPIO_InitStruct );

  GPIO_InitStruct.Pin = GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_14
                        | GPIO_PIN_15 | GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_3
                        | GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_7;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF12_FMC;
  HAL_GPIO_Init( GPIOD, &GPIO_InitStruct );

  GPIO_InitStruct.Pin = GPIO_PIN_7;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF12_FMC;
  HAL_GPIO_Init( GPIOB, &GPIO_InitStruct );

  /* USER CODE BEGIN FMC_MspInit 1 */

  /* USER CODE END FMC_MspInit 1 */
}

void HAL_SRAM_MspInit( SRAM_HandleTypeDef *hsram )
{
  /* USER CODE BEGIN SRAM_MspInit 0 */

  /* USER CODE END SRAM_MspInit 0 */
  HAL_FMC_MspInit();
  /* USER CODE BEGIN SRAM_MspInit 1 */

  /* USER CODE END SRAM_MspInit 1 */
}

static uint32_t FMC_DeInitialized = 0;

static void HAL_FMC_MspDeInit( void )
{
  /* USER CODE BEGIN FMC_MspDeInit 0 */

  /* USER CODE END FMC_MspDeInit 0 */
  if( FMC_DeInitialized ) {
    return;
  }
  FMC_DeInitialized = 1;
  /* Peripheral clock enable */
  __HAL_RCC_FMC_CLK_DISABLE();

  /** FMC GPIO Configuration
  PF0   ------> FMC_A0
  PF1   ------> FMC_A1
  PF2   ------> FMC_A2
  PF3   ------> FMC_A3
  PF4   ------> FMC_A4
  PF5   ------> FMC_A5
  PF12   ------> FMC_A6
  PF13   ------> FMC_A7
  PF14   ------> FMC_A8
  PF15   ------> FMC_A9
  PG0   ------> FMC_A10
  PG1   ------> FMC_A11
  PE7   ------> FMC_D4
  PE8   ------> FMC_D5
  PE9   ------> FMC_D6
  PE10   ------> FMC_D7
  PE11   ------> FMC_D8
  PE12   ------> FMC_D9
  PE13   ------> FMC_D10
  PE14   ------> FMC_D11
  PE15   ------> FMC_D12
  PD8   ------> FMC_D13
  PD9   ------> FMC_D14
  PD10   ------> FMC_D15
  PD14   ------> FMC_D0
  PD15   ------> FMC_D1
  PG2   ------> FMC_A12
  PG3   ------> FMC_A13
  PG4   ------> FMC_A14
  PG5   ------> FMC_A15
  PD0   ------> FMC_D2
  PD1   ------> FMC_D3
  PD3   ------> FMC_CLK
  PD4   ------> FMC_NOE
  PD5   ------> FMC_NWE
  PD7   ------> FMC_NE1
  PB7   ------> FMC_NL
  */
  HAL_GPIO_DeInit( GPIOG, GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3
                   | GPIO_PIN_4 | GPIO_PIN_5 );

  HAL_GPIO_DeInit( GPIOE, GPIO_PIN_7 | GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10
                   | GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14
                   | GPIO_PIN_15 );

  HAL_GPIO_DeInit( GPIOD, GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_14
                   | GPIO_PIN_15 | GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_3
                   | GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_7 );

  HAL_GPIO_DeInit( GPIOB, GPIO_PIN_7 );

  /* USER CODE BEGIN FMC_MspDeInit 1 */

  /* USER CODE END FMC_MspDeInit 1 */
}
void HAL_SRAM_MspDeInit( SRAM_HandleTypeDef *hsram )
{
  /* USER CODE BEGIN SRAM_MspDeInit 0 */

  /* USER CODE END SRAM_MspDeInit 0 */
  HAL_FMC_MspDeInit();
  /* USER CODE BEGIN SRAM_MspDeInit 1 */

  /* USER CODE END SRAM_MspDeInit 1 */
}

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */

/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
