/**
  ******************************************************************************
  * @file    usart.c
  * @brief   This file provides code for the configuration
  *          of the USART instances.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "usart.h"

/* USER CODE BEGIN 0 */
#include "stdio.h"
#include "string.h"

#define UART_RING_BUF_SIZE   1024   /* 环形缓冲区大小，必须是2的幂 */
#define UART_RING_BUF_MASK   (UART_RING_BUF_SIZE - 1)  /* 位掩码，用于取模 */

uint8_t uart_recv_arr[8];          /*串口接收缓冲区（帧头+数据+帧尾）*/
volatile uint8_t uart_recv_flag=0;           /*串口接收完成标志位*/

/* 环形缓冲区（带溢出保护） */
static uint8_t uart_ring_buf[UART_RING_BUF_SIZE];
static volatile uint16_t uart_ring_head = 0;  /* 写入位置（生产者） */
static volatile uint16_t uart_ring_tail = 0;  /* 读取位置（消费者） */

/* 单字节接收缓冲 */
static uint8_t uart_rx_byte;
/* 如果使用os,则包括下面的头文件即可. */
#if SYS_SUPPORT_OS
#include "os.h" /* os 使用 */
#endif

/******************************************************************************************/
/* 加入以下代码, 支持printf函数, 而不需要选择use MicroLIB */

#if 1

#if (__ARMCC_VERSION >= 6010050)            /* 使用AC6编译器时 */
__asm(".global __use_no_semihosting\n\t");  /* 声明不使用半主机模式 */
__asm(".global __ARM_use_no_argv \n\t");    /* AC6下需要声明main函数为无参数格式，否则部分例程可能出现半主机模式 */

#else
/* 使用AC5编译器时, 要在这里定义__FILE 和 不使用半主机模式 */
#pragma import(__use_no_semihosting)

struct __FILE
{
    int handle;
    /* Whatever you require here. If the only file you are using is */
    /* standard output using printf() for debugging, no file handling */
    /* is required. */
};

#endif

/* 不使用半主机模式，至少需要重定义_ttywrch\_sys_exit\_sys_command_string函数,以同时兼容AC6和AC5模式 */
int _ttywrch(int ch)
{
    ch = ch;
    return ch;
}

/* 定义_sys_exit()以避免使用半主机模式 */
void _sys_exit(int x)
{
    x = x;
}

char *_sys_command_string(char *cmd, int len)
{
    return NULL;
}


/* FILE 在 stdio.h里面定义. */
FILE __stdout;

/* MDK下需要重定义fputc函数, printf函数最终会通过调用fputc输出字符串到串口 */
int fputc(int ch, FILE *f)
{
     while (!(USART1->ISR&0x40));           /* 等待上一个字符发送完成 */

    USART1->TDR = (uint8_t)ch;             /* 将要发送的字符 ch 写入到DR寄存器 */
    return ch;
}
#endif


/* USER CODE END 0 */

UART_HandleTypeDef huart1;

/* USART1 init function */

void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  huart1.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart1.Init.ClockPrescaler = UART_PRESCALER_DIV1;
  huart1.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetTxFifoThreshold(&huart1, UART_TXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetRxFifoThreshold(&huart1, UART_RXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_DisableFifoMode(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */
   HAL_UART_Receive_IT(&huart1, &uart_rx_byte, 1);  /* 每次接收1个字节 */
  /* USER CODE END USART1_Init 2 */

}

void HAL_UART_MspInit(UART_HandleTypeDef* uartHandle)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if(uartHandle->Instance==USART1)
  {
  /* USER CODE BEGIN USART1_MspInit 0 */

  /* USER CODE END USART1_MspInit 0 */
    /* USART1 clock enable */
    __HAL_RCC_USART1_CLK_ENABLE();

    __HAL_RCC_GPIOB_CLK_ENABLE();
    /**USART1 GPIO Configuration
    PB6     ------> USART1_TX
    PB7     ------> USART1_RX
    */
    GPIO_InitStruct.Pin = GPIO_PIN_6|GPIO_PIN_7;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF0_USART1;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    /* USART1 interrupt Init */
    HAL_NVIC_SetPriority(USART1_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(USART1_IRQn);
  /* USER CODE BEGIN USART1_MspInit 1 */

  /* USER CODE END USART1_MspInit 1 */
  }
}

void HAL_UART_MspDeInit(UART_HandleTypeDef* uartHandle)
{

  if(uartHandle->Instance==USART1)
  {
  /* USER CODE BEGIN USART1_MspDeInit 0 */

  /* USER CODE END USART1_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_USART1_CLK_DISABLE();

    /**USART1 GPIO Configuration
    PB6     ------> USART1_TX
    PB7     ------> USART1_RX
    */
    HAL_GPIO_DeInit(GPIOB, GPIO_PIN_6|GPIO_PIN_7);

    /* USART1 interrupt Deinit */
    HAL_NVIC_DisableIRQ(USART1_IRQn);
  /* USER CODE BEGIN USART1_MspDeInit 1 */

  /* USER CODE END USART1_MspDeInit 1 */
  }
}

/* USER CODE BEGIN 1 */

/**
 * @brief  获取环形缓冲区已使用空间
 * @retval 数据数量（0 ~ UART_RING_BUF_SIZE）
 */
static inline uint16_t uart_ring_used(void)
{
    uint16_t used = uart_ring_head - uart_ring_tail;
    /* 防御性检查：理论上不会超过 BUF_SIZE，但防止异常情况 */
    if(used > UART_RING_BUF_SIZE)
        return 0;
    return used;
}

/**
 * @brief  获取环形缓冲区剩余空间
 * @retval 剩余可写入的字节数
 */
static inline uint16_t uart_ring_free(void)
{
    return UART_RING_BUF_SIZE - uart_ring_used();
}

/**
 * @brief  将数据写入环形缓冲区（满时覆盖最旧数据）
 * @param  data: 要写入的数据
 * @retval 始终返回0（写入成功）
 */
static int uart_ring_write(uint8_t data)
{
    /* 缓冲区满时，移动tail丢弃最旧的数据 */
    if(uart_ring_free() == 0)
    {
        uart_ring_tail++;  /* 丢弃最旧的数据 */
    }

    uart_ring_buf[uart_ring_head & UART_RING_BUF_MASK] = data;
    uart_ring_head++;
    return 0;
}

/**
 * @brief  从环形缓冲区读取数据
 * @param  data: 读取数据的指针，为NULL时仅丢弃
 * @retval 0: 成功, -1: 缓冲区为空
 */
static int uart_ring_read(uint8_t *data)
{
    if(uart_ring_used() == 0)
        return -1;

    if(data != NULL)
        *data = uart_ring_buf[uart_ring_tail & UART_RING_BUF_MASK];
    uart_ring_tail++;
    return 0;
}

/**
 * @brief  从环形缓冲区中预读数据（不移动读指针）
 * @param  offset: 偏移量（0表示最旧的数据）
 * @param  data: 读取数据的指针
 * @retval 0: 成功, -1: 超出范围
 */
static int uart_ring_peek(uint16_t offset, uint8_t *data)
{
    if(offset >= uart_ring_used())
        return -1;
    *data = uart_ring_buf[(uart_ring_tail + offset) & UART_RING_BUF_MASK];
    return 0;
}

/**
 * @brief  串口接收完成回调函数（环形缓冲区 + 帧解析）
 * @note   帧格式: [0xFE] [data0] [data1] [data2] [data3] [data4] [data5] [0xFF]
 *                 帧头(1字节) + 数据(6字节) + 帧尾(1字节) = 8字节
 *
 *         处理流程:
 *         1. 将接收到的字节存入环形缓冲区
 *         2. 从缓冲区中解析完整帧
 *         3. 找到帧头0xFE后，检查是否有8字节
 *         4. 验证帧尾0xFF，存储有效帧
 */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if(huart->Instance == USART1)
    {
        /* 1. 将接收到的字节存入环形缓冲区 */
        uart_ring_write(uart_rx_byte);

        /* 2. 从缓冲区解析帧 */
        while(uart_ring_used() >= 8)
        {
            uint8_t header, tail;

            /* 预读第0字节，检查帧头 */
            uart_ring_peek(0, &header);
            if(header != 0xFE)
            {
                /* 帧头错误，丢弃这个字节，继续查找 */
                uart_ring_read(NULL);
                continue;
            }

            /* 预读第7字节，检查帧尾 */
            uart_ring_peek(7, &tail);
            if(tail != 0xFF)
            {
                /* 帧尾错误，丢弃帧头，继续查找 */
                uart_ring_read(NULL);
                continue;
            }

            /* 找到完整帧，读取并存储 */
            for(int i = 0; i < 8; i++)
            {
                uart_ring_read(&uart_recv_arr[i]);
            }
            uart_recv_flag = 1;  /* 标记接收完成 */
            break;  /* 每次中断只处理一帧 */
        }

        /* 重新启动单字节接收 */
        HAL_UART_Receive_IT(&huart1, &uart_rx_byte, 1);
    }
}
/* USER CODE END 1 */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
