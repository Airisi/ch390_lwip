/********************************** (C) COPYRIGHT *****************************
 * File Name          : CH390_Interface.c
 * Author             : WCH
 * Version            : V1.0
 * Date               : 2024/08/20
 * Description        : CH390 interface
 ******************************************************************************
 * Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
 * Attention: This software (modified or not) and binary are used for
 * microcontroller manufactured by Nanjing Qinheng Microelectronics.
 ******************************************************************************/
#include "main.h"
#include "spi.h"
#include "CH390_Interface.h"
#include "delay.h"
#include "usb_diag.h"

static void select_chip(GPIO_PinState state)
{
    HAL_GPIO_WritePin(CH390_CS_GPIO_Port, CH390_CS_Pin, state);
}
static uint8_t exchange(uint8_t byte)
{
    uint8_t value = 0xFF;
    HAL_StatusTypeDef status;
#if USB_DIAG_ENABLE
    uint32_t start = HAL_GetTick();
#endif
    if (ch390_get_error() != CH390_ERROR_NONE) { return value; }
    status = HAL_SPI_TransmitReceive(&hspi2, &byte, &value, 1, 10);
#if USB_DIAG_ENABLE
    {
        uint32_t elapsed = HAL_GetTick() - start;
        if (elapsed > usb_diag.spi_max_ms) { usb_diag.spi_max_ms = elapsed; }
        if (status != HAL_OK) { usb_diag.spi_status = (uint8_t)status; ++usb_diag.spi_errors; }
    }
#endif
    if (status != HAL_OK) {
        select_chip(GPIO_PIN_SET);
        ch390_set_error(CH390_ERROR_SPI);
    }
    return value;
}
uint32_t ch390_get_tick(void) { return HAL_GetTick(); }
uint16_t ch390_get_int_pin(void) { return HAL_GPIO_ReadPin(CH390_INT_GPIO_Port, CH390_INT_Pin) == GPIO_PIN_SET; }
void ch390_delay_us(uint32_t us) { delay_us(us); }
void ch30_delay_ns(uint32_t cycles) { while (cycles-- != 0) { __NOP(); } }
uint8_t ch390_read_reg(uint8_t reg)
{
    uint8_t value;
    if (ch390_get_error() != CH390_ERROR_NONE) { return 0xFF; }
    select_chip(GPIO_PIN_RESET);
    exchange(reg | OPC_REG_R);
    value = exchange(0);
    select_chip(GPIO_PIN_SET);
#if USB_DIAG_ENABLE
    usb_diag.last_reg = reg;
    usb_diag.last_value = value;
    if (reg == CH390_NSR) { usb_diag.nsr = value; }
#endif
    return value;
}
void ch390_write_reg(uint8_t reg, uint8_t value)
{
    if (ch390_get_error() != CH390_ERROR_NONE) { return; }
    select_chip(GPIO_PIN_RESET);
    exchange(reg | OPC_REG_W);
    exchange(value);
    select_chip(GPIO_PIN_SET);
}
void ch390_read_mem(uint8_t *data, int length)
{
    if (ch390_get_error() != CH390_ERROR_NONE) { return; }
    select_chip(GPIO_PIN_RESET);
    exchange(OPC_MEM_READ);
    while (length-- > 0 && ch390_get_error() == CH390_ERROR_NONE) { *data++ = exchange(0); }
    select_chip(GPIO_PIN_SET);
}
void ch390_write_mem(uint8_t *data, int length)
{
    if (ch390_get_error() != CH390_ERROR_NONE) { return; }
    select_chip(GPIO_PIN_RESET);
    exchange(OPC_MEM_WRITE);
    while (length-- > 0 && ch390_get_error() == CH390_ERROR_NONE) { exchange(*data++); }
    select_chip(GPIO_PIN_SET);
}


