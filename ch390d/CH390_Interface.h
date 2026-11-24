/********************************** (C) COPYRIGHT *******************************
* File Name          : CH390.h
* Author             : WCH
* Version            : V1.0
* Date               : 2024/08/20
* Description        : CH390 interface header file
*******************************************************************************/

#ifndef __CH390_INTERFACE_H
#define __CH390_INTERFACE_H

#include <stdint.h>
#include "ch390.h"

//void ch390_gpio_init(void);


enum { CH390_ERROR_NONE, CH390_ERROR_SPI, CH390_ERROR_BUSY, CH390_ERROR_ID, CH390_ERROR_MAC };
uint8_t ch390_get_error(void);
void ch390_clear_error(void);
void ch390_set_error(uint8_t error);
uint32_t ch390_get_tick(void);
uint16_t ch390_get_int_pin(void);
void ch390_delay_us(uint32_t time);
void ch30_delay_ns(uint32_t ns);

/**
 * @name ch390_read_reg
 * @brief Read register
 * @param reg - Target register address
 * @return Register value
 */
uint8_t ch390_read_reg(uint8_t reg);

/**
 * @name ch390_write_reg
 * @brief Write register
 * @param reg - Target register address
 * @param value - Value to be written
 */
void ch390_write_reg(uint8_t reg, uint8_t value);

/**
 * @name ch390_read_mem
 * @brief Read data from RX SRAM
 * @param data - Data buffer
 * @param length - Length to read
 */
void ch390_read_mem(uint8_t *data, int length);

/**
 * @name ch390_write_mem
 * @brief Write data to TX SRAM
 * @param data - Data buffer
 * @param length - Length to write
 */
void ch390_write_mem(uint8_t *data, int length);

#endif /* __CH390_INTERFACE_H */
