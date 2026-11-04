/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    gpio.c
  * @brief   This file provides code for the configuration
  *          of all used GPIO pins.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "gpio.h"

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/*----------------------------------------------------------------------------*/
/* Configure GPIO                                                             */
/*----------------------------------------------------------------------------*/
/* USER CODE BEGIN 1 */

/* USER CODE END 1 */

/** Configure pins as
        * Analog
        * Input
        * Output
        * EVENT_OUT
        * EXTI
*/
void MX_GPIO_Init(void)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOE_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOF_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOE, OUT_PE02_Pin|OUT_PE03_Pin|OUT_PE04_Pin|OUT_PE05_Pin
                          |OUT_PE06_Pin|OUT_PE07_Pin|OUT_PE08_Pin|OUT_PE09_Pin
                          |OUT_PE10_Pin|OUT_PE11_Pin|OUT_PE12_Pin|OUT_PE13_Pin
                          |OUT_PE14_Pin|OUT_PE15_Pin|OUT_PE00_Pin|OUT_PE01_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, OUT_PC13_Pin|OUT_PC14_Pin|OUT_PC15_Pin|OUT_PC00_Pin
                          |OUT_PC01_Pin|OUT_PC02_Pin|OUT_PC03_Pin|OUT_PC04_Pin
                          |OUT_PC05_Pin|OUT_PC06_Pin|OUT_PC07_Pin|OUT_PC08_Pin
                          |OUT_PC09_Pin|OUT_PC10_Pin|OUT_PC11_Pin|OUT_PC12_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOF, OUT_PF09_Pin|OUT_PF10_Pin|OUT_PF00_Pin|OUT_PF01_Pin
                          |OUT_PF02_Pin|OUT_PF03_Pin|OUT_PF06_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, OUT_PA00_Pin|OUT_PA01_Pin|OUT_PA02_Pin|OUT_PA03_Pin
                          |OUT_PA04_Pin|OUT_PA05_Pin|OUT_PA06_Pin|OUT_PA07_Pin
                          |OUT_PA08_Pin|OUT_PA09_Pin|OUT_PA10_Pin|OUT_PA15_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, OUT_PB00_Pin|OUT_PB01_Pin|OUT_PB02_Pin|OUT_PB10_Pin
                          |OUT_PB11_Pin|OUT_PB03_Pin|OUT_PB04_Pin|OUT_PB05_Pin
                          |OUT_PB06_Pin|OUT_PB07_Pin|OUT_PB08_Pin|OUT_PB09_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(CH390_CS_GPIO_Port, CH390_CS_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOD, OUT_PD09_Pin|OUT_PD10_Pin|OUT_PD11_Pin|OUT_PD12_Pin
                          |OUT_PD13_Pin|OUT_PD14_Pin|OUT_PD15_Pin|OUT_PD00_Pin
                          |OUT_PD01_Pin|OUT_PD02_Pin|OUT_PD03_Pin|OUT_PD04_Pin
                          |OUT_PD05_Pin|OUT_PD06_Pin|OUT_PD07_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pins : OUT_PE02_Pin OUT_PE03_Pin OUT_PE04_Pin OUT_PE05_Pin
                           OUT_PE06_Pin OUT_PE07_Pin OUT_PE08_Pin OUT_PE09_Pin
                           OUT_PE10_Pin OUT_PE11_Pin OUT_PE12_Pin OUT_PE13_Pin
                           OUT_PE14_Pin OUT_PE15_Pin OUT_PE00_Pin OUT_PE01_Pin */
  GPIO_InitStruct.Pin = OUT_PE02_Pin|OUT_PE03_Pin|OUT_PE04_Pin|OUT_PE05_Pin
                          |OUT_PE06_Pin|OUT_PE07_Pin|OUT_PE08_Pin|OUT_PE09_Pin
                          |OUT_PE10_Pin|OUT_PE11_Pin|OUT_PE12_Pin|OUT_PE13_Pin
                          |OUT_PE14_Pin|OUT_PE15_Pin|OUT_PE00_Pin|OUT_PE01_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

  /*Configure GPIO pins : OUT_PC13_Pin OUT_PC14_Pin OUT_PC15_Pin OUT_PC00_Pin
                           OUT_PC01_Pin OUT_PC02_Pin OUT_PC03_Pin OUT_PC04_Pin
                           OUT_PC05_Pin OUT_PC06_Pin OUT_PC07_Pin OUT_PC08_Pin
                           OUT_PC09_Pin OUT_PC10_Pin OUT_PC11_Pin OUT_PC12_Pin */
  GPIO_InitStruct.Pin = OUT_PC13_Pin|OUT_PC14_Pin|OUT_PC15_Pin|OUT_PC00_Pin
                          |OUT_PC01_Pin|OUT_PC02_Pin|OUT_PC03_Pin|OUT_PC04_Pin
                          |OUT_PC05_Pin|OUT_PC06_Pin|OUT_PC07_Pin|OUT_PC08_Pin
                          |OUT_PC09_Pin|OUT_PC10_Pin|OUT_PC11_Pin|OUT_PC12_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : OUT_PF09_Pin OUT_PF10_Pin OUT_PF00_Pin OUT_PF01_Pin
                           OUT_PF02_Pin OUT_PF03_Pin OUT_PF06_Pin */
  GPIO_InitStruct.Pin = OUT_PF09_Pin|OUT_PF10_Pin|OUT_PF00_Pin|OUT_PF01_Pin
                          |OUT_PF02_Pin|OUT_PF03_Pin|OUT_PF06_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);

  /*Configure GPIO pins : OUT_PA00_Pin OUT_PA01_Pin OUT_PA02_Pin OUT_PA03_Pin
                           OUT_PA04_Pin OUT_PA05_Pin OUT_PA06_Pin OUT_PA07_Pin
                           OUT_PA08_Pin OUT_PA09_Pin OUT_PA10_Pin OUT_PA15_Pin */
  GPIO_InitStruct.Pin = OUT_PA00_Pin|OUT_PA01_Pin|OUT_PA02_Pin|OUT_PA03_Pin
                          |OUT_PA04_Pin|OUT_PA05_Pin|OUT_PA06_Pin|OUT_PA07_Pin
                          |OUT_PA08_Pin|OUT_PA09_Pin|OUT_PA10_Pin|OUT_PA15_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : OUT_PB00_Pin OUT_PB01_Pin OUT_PB02_Pin OUT_PB10_Pin
                           OUT_PB11_Pin OUT_PB03_Pin OUT_PB04_Pin OUT_PB05_Pin
                           OUT_PB06_Pin OUT_PB07_Pin OUT_PB08_Pin OUT_PB09_Pin */
  GPIO_InitStruct.Pin = OUT_PB00_Pin|OUT_PB01_Pin|OUT_PB02_Pin|OUT_PB10_Pin
                          |OUT_PB11_Pin|OUT_PB03_Pin|OUT_PB04_Pin|OUT_PB05_Pin
                          |OUT_PB06_Pin|OUT_PB07_Pin|OUT_PB08_Pin|OUT_PB09_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : CH390_CS_Pin */
  GPIO_InitStruct.Pin = CH390_CS_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(CH390_CS_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : CH390_INT_Pin */
  GPIO_InitStruct.Pin = CH390_INT_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  HAL_GPIO_Init(CH390_INT_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : OUT_PD09_Pin OUT_PD10_Pin OUT_PD11_Pin OUT_PD12_Pin
                           OUT_PD13_Pin OUT_PD14_Pin OUT_PD15_Pin OUT_PD00_Pin
                           OUT_PD01_Pin OUT_PD02_Pin OUT_PD03_Pin OUT_PD04_Pin
                           OUT_PD05_Pin OUT_PD06_Pin OUT_PD07_Pin */
  GPIO_InitStruct.Pin = OUT_PD09_Pin|OUT_PD10_Pin|OUT_PD11_Pin|OUT_PD12_Pin
                          |OUT_PD13_Pin|OUT_PD14_Pin|OUT_PD15_Pin|OUT_PD00_Pin
                          |OUT_PD01_Pin|OUT_PD02_Pin|OUT_PD03_Pin|OUT_PD04_Pin
                          |OUT_PD05_Pin|OUT_PD06_Pin|OUT_PD07_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

}

/* USER CODE BEGIN 2 */

/* USER CODE END 2 */
