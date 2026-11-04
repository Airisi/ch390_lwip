/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f0xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define OUT_PE02_Pin GPIO_PIN_2
#define OUT_PE02_GPIO_Port GPIOE
#define OUT_PE03_Pin GPIO_PIN_3
#define OUT_PE03_GPIO_Port GPIOE
#define OUT_PE04_Pin GPIO_PIN_4
#define OUT_PE04_GPIO_Port GPIOE
#define OUT_PE05_Pin GPIO_PIN_5
#define OUT_PE05_GPIO_Port GPIOE
#define OUT_PE06_Pin GPIO_PIN_6
#define OUT_PE06_GPIO_Port GPIOE
#define OUT_PC13_Pin GPIO_PIN_13
#define OUT_PC13_GPIO_Port GPIOC
#define OUT_PC14_Pin GPIO_PIN_14
#define OUT_PC14_GPIO_Port GPIOC
#define OUT_PC15_Pin GPIO_PIN_15
#define OUT_PC15_GPIO_Port GPIOC
#define OUT_PF09_Pin GPIO_PIN_9
#define OUT_PF09_GPIO_Port GPIOF
#define OUT_PF10_Pin GPIO_PIN_10
#define OUT_PF10_GPIO_Port GPIOF
#define OUT_PF00_Pin GPIO_PIN_0
#define OUT_PF00_GPIO_Port GPIOF
#define OUT_PF01_Pin GPIO_PIN_1
#define OUT_PF01_GPIO_Port GPIOF
#define OUT_PC00_Pin GPIO_PIN_0
#define OUT_PC00_GPIO_Port GPIOC
#define OUT_PC01_Pin GPIO_PIN_1
#define OUT_PC01_GPIO_Port GPIOC
#define OUT_PC02_Pin GPIO_PIN_2
#define OUT_PC02_GPIO_Port GPIOC
#define OUT_PC03_Pin GPIO_PIN_3
#define OUT_PC03_GPIO_Port GPIOC
#define OUT_PF02_Pin GPIO_PIN_2
#define OUT_PF02_GPIO_Port GPIOF
#define OUT_PF03_Pin GPIO_PIN_3
#define OUT_PF03_GPIO_Port GPIOF
#define OUT_PA00_Pin GPIO_PIN_0
#define OUT_PA00_GPIO_Port GPIOA
#define OUT_PA01_Pin GPIO_PIN_1
#define OUT_PA01_GPIO_Port GPIOA
#define OUT_PA02_Pin GPIO_PIN_2
#define OUT_PA02_GPIO_Port GPIOA
#define OUT_PA03_Pin GPIO_PIN_3
#define OUT_PA03_GPIO_Port GPIOA
#define OUT_PA04_Pin GPIO_PIN_4
#define OUT_PA04_GPIO_Port GPIOA
#define OUT_PA05_Pin GPIO_PIN_5
#define OUT_PA05_GPIO_Port GPIOA
#define OUT_PA06_Pin GPIO_PIN_6
#define OUT_PA06_GPIO_Port GPIOA
#define OUT_PA07_Pin GPIO_PIN_7
#define OUT_PA07_GPIO_Port GPIOA
#define OUT_PC04_Pin GPIO_PIN_4
#define OUT_PC04_GPIO_Port GPIOC
#define OUT_PC05_Pin GPIO_PIN_5
#define OUT_PC05_GPIO_Port GPIOC
#define OUT_PB00_Pin GPIO_PIN_0
#define OUT_PB00_GPIO_Port GPIOB
#define OUT_PB01_Pin GPIO_PIN_1
#define OUT_PB01_GPIO_Port GPIOB
#define OUT_PB02_Pin GPIO_PIN_2
#define OUT_PB02_GPIO_Port GPIOB
#define OUT_PE07_Pin GPIO_PIN_7
#define OUT_PE07_GPIO_Port GPIOE
#define OUT_PE08_Pin GPIO_PIN_8
#define OUT_PE08_GPIO_Port GPIOE
#define OUT_PE09_Pin GPIO_PIN_9
#define OUT_PE09_GPIO_Port GPIOE
#define OUT_PE10_Pin GPIO_PIN_10
#define OUT_PE10_GPIO_Port GPIOE
#define OUT_PE11_Pin GPIO_PIN_11
#define OUT_PE11_GPIO_Port GPIOE
#define OUT_PE12_Pin GPIO_PIN_12
#define OUT_PE12_GPIO_Port GPIOE
#define OUT_PE13_Pin GPIO_PIN_13
#define OUT_PE13_GPIO_Port GPIOE
#define OUT_PE14_Pin GPIO_PIN_14
#define OUT_PE14_GPIO_Port GPIOE
#define OUT_PE15_Pin GPIO_PIN_15
#define OUT_PE15_GPIO_Port GPIOE
#define OUT_PB10_Pin GPIO_PIN_10
#define OUT_PB10_GPIO_Port GPIOB
#define OUT_PB11_Pin GPIO_PIN_11
#define OUT_PB11_GPIO_Port GPIOB
#define CH390_CS_Pin GPIO_PIN_12
#define CH390_CS_GPIO_Port GPIOB
#define CH390_INT_Pin GPIO_PIN_8
#define CH390_INT_GPIO_Port GPIOD
#define OUT_PD09_Pin GPIO_PIN_9
#define OUT_PD09_GPIO_Port GPIOD
#define OUT_PD10_Pin GPIO_PIN_10
#define OUT_PD10_GPIO_Port GPIOD
#define OUT_PD11_Pin GPIO_PIN_11
#define OUT_PD11_GPIO_Port GPIOD
#define OUT_PD12_Pin GPIO_PIN_12
#define OUT_PD12_GPIO_Port GPIOD
#define OUT_PD13_Pin GPIO_PIN_13
#define OUT_PD13_GPIO_Port GPIOD
#define OUT_PD14_Pin GPIO_PIN_14
#define OUT_PD14_GPIO_Port GPIOD
#define OUT_PD15_Pin GPIO_PIN_15
#define OUT_PD15_GPIO_Port GPIOD
#define OUT_PC06_Pin GPIO_PIN_6
#define OUT_PC06_GPIO_Port GPIOC
#define OUT_PC07_Pin GPIO_PIN_7
#define OUT_PC07_GPIO_Port GPIOC
#define OUT_PC08_Pin GPIO_PIN_8
#define OUT_PC08_GPIO_Port GPIOC
#define OUT_PC09_Pin GPIO_PIN_9
#define OUT_PC09_GPIO_Port GPIOC
#define OUT_PA08_Pin GPIO_PIN_8
#define OUT_PA08_GPIO_Port GPIOA
#define OUT_PA09_Pin GPIO_PIN_9
#define OUT_PA09_GPIO_Port GPIOA
#define OUT_PA10_Pin GPIO_PIN_10
#define OUT_PA10_GPIO_Port GPIOA
#define OUT_PF06_Pin GPIO_PIN_6
#define OUT_PF06_GPIO_Port GPIOF
#define OUT_PA15_Pin GPIO_PIN_15
#define OUT_PA15_GPIO_Port GPIOA
#define OUT_PC10_Pin GPIO_PIN_10
#define OUT_PC10_GPIO_Port GPIOC
#define OUT_PC11_Pin GPIO_PIN_11
#define OUT_PC11_GPIO_Port GPIOC
#define OUT_PC12_Pin GPIO_PIN_12
#define OUT_PC12_GPIO_Port GPIOC
#define OUT_PD00_Pin GPIO_PIN_0
#define OUT_PD00_GPIO_Port GPIOD
#define OUT_PD01_Pin GPIO_PIN_1
#define OUT_PD01_GPIO_Port GPIOD
#define OUT_PD02_Pin GPIO_PIN_2
#define OUT_PD02_GPIO_Port GPIOD
#define OUT_PD03_Pin GPIO_PIN_3
#define OUT_PD03_GPIO_Port GPIOD
#define OUT_PD04_Pin GPIO_PIN_4
#define OUT_PD04_GPIO_Port GPIOD
#define OUT_PD05_Pin GPIO_PIN_5
#define OUT_PD05_GPIO_Port GPIOD
#define OUT_PD06_Pin GPIO_PIN_6
#define OUT_PD06_GPIO_Port GPIOD
#define OUT_PD07_Pin GPIO_PIN_7
#define OUT_PD07_GPIO_Port GPIOD
#define OUT_PB03_Pin GPIO_PIN_3
#define OUT_PB03_GPIO_Port GPIOB
#define OUT_PB04_Pin GPIO_PIN_4
#define OUT_PB04_GPIO_Port GPIOB
#define OUT_PB05_Pin GPIO_PIN_5
#define OUT_PB05_GPIO_Port GPIOB
#define OUT_PB06_Pin GPIO_PIN_6
#define OUT_PB06_GPIO_Port GPIOB
#define OUT_PB07_Pin GPIO_PIN_7
#define OUT_PB07_GPIO_Port GPIOB
#define OUT_PB08_Pin GPIO_PIN_8
#define OUT_PB08_GPIO_Port GPIOB
#define OUT_PB09_Pin GPIO_PIN_9
#define OUT_PB09_GPIO_Port GPIOB
#define OUT_PE00_Pin GPIO_PIN_0
#define OUT_PE00_GPIO_Port GPIOE
#define OUT_PE01_Pin GPIO_PIN_1
#define OUT_PE01_GPIO_Port GPIOE

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
