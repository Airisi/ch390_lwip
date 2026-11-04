#include "main.h"
#include "spi.h"
#include "CH390_Interface.h"
#include "delay.h"

static void chip_select(GPIO_PinState state)
{
    HAL_GPIO_WritePin(CH390_CS_GPIO_Port, CH390_CS_Pin, state);
}

static uint8_t exchange(uint8_t byte)
{
    uint8_t value = 0;
    if (HAL_SPI_TransmitReceive(&hspi2, &byte, &value, 1, 10) != HAL_OK) {
        chip_select(GPIO_PIN_SET);
        Error_Handler();
    }
    return value;
}

uint16_t ch390_get_int_pin(void)
{
    return HAL_GPIO_ReadPin(CH390_INT_GPIO_Port, CH390_INT_Pin) == GPIO_PIN_SET;
}

void ch390_delay_us(uint32_t us) { delay_us(us); }
void ch30_delay_ns(uint32_t cycles) { while (cycles-- != 0U) { __NOP(); } }

uint8_t ch390_read_reg(uint8_t reg)
{
    uint8_t value;
    chip_select(GPIO_PIN_RESET);
    exchange(reg | OPC_REG_R);
    value = exchange(0);
    chip_select(GPIO_PIN_SET);
    return value;
}

void ch390_write_reg(uint8_t reg, uint8_t value)
{
    chip_select(GPIO_PIN_RESET);
    exchange(reg | OPC_REG_W);
    exchange(value);
    chip_select(GPIO_PIN_SET);
}

void ch390_read_mem(uint8_t *data, int length)
{
    chip_select(GPIO_PIN_RESET);
    exchange(OPC_MEM_READ);
    while (length-- > 0) { *data++ = exchange(0); }
    chip_select(GPIO_PIN_SET);
}

void ch390_write_mem(uint8_t *data, int length)
{
    chip_select(GPIO_PIN_RESET);
    exchange(OPC_MEM_WRITE);
    while (length-- > 0) { exchange(*data++); }
    chip_select(GPIO_PIN_SET);
}
