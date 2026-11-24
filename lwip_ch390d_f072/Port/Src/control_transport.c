/* UART-to-USB platform adapter. RX framing follows the original UART parser. */
#include "control_transport.h"
#include "main.h"
#include "usb_device.h"
#include "usbd_cdc_if.h"
#include "usbd_cdc.h"
#include <stddef.h>
#include "usb_diag.h"
#include <stdio.h>

extern USBD_HandleTypeDef hUsbDeviceFS;
/* Sized for the F072's 16 KB SRAM; parser behavior is unchanged. */
#define UART_RING_BUF_SIZE 256U
#define UART_RING_BUF_MASK (UART_RING_BUF_SIZE - 1U)
#define TX_SIZE 256U
static uint8_t uart_ring_buf[UART_RING_BUF_SIZE];
static volatile uint16_t uart_ring_head, uart_ring_tail;
uint8_t uart_recv_arr[8];
volatile uint8_t uart_recv_flag;
static uint8_t tx_ring[TX_SIZE], tx_packet[64];
static uint16_t tx_head, tx_tail;
#define DEBUG_REPLY_RESERVE 32U
#if USB_DIAG_ENABLE
volatile UsbDiagnostics usb_diag;
static uint32_t report_tick, busy_tick;
static uint8_t report_line, busy_active;
#endif
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


void control_transport_usb_rx(const uint8_t *data, uint32_t length)
{
    uint32_t index;
#if USB_DIAG_ENABLE
    ++usb_diag.rx_packets;
    usb_diag.rx_bytes += length;
    usb_diag.last_rx_ms = HAL_GetTick();
    usb_diag.last_rx_length = (uint16_t)length;
    for (index = 0; index < sizeof(usb_diag.last_rx); ++index) {
        usb_diag.last_rx[index] = index < length ? data[index] : 0;
    }
#endif
    for (index = 0; index < length; ++index) {
        uint8_t uart_rx_byte = data[index];
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
#if USB_DIAG_ENABLE
                ++usb_diag.bad_bytes;
#endif
                uart_ring_read(NULL);
                continue;
            }

            /* 预读第7字节，检查帧尾 */
            uart_ring_peek(7, &tail);
            if(tail != 0xFF)
            {
                /* 帧尾错误，丢弃帧头，继续查找 */
#if USB_DIAG_ENABLE
                ++usb_diag.bad_bytes;
#endif
                uart_ring_read(NULL);
                continue;
            }

#if USB_DIAG_ENABLE
            ++usb_diag.frames;
            usb_diag.last_frame_ms = HAL_GetTick();
            if (uart_recv_flag) { ++usb_diag.overwrites; }
#endif
            /* 找到完整帧，读取并存储 */
            for(int i = 0; i < 8; i++)
            {
                uart_ring_read(&uart_recv_arr[i]);
            }
            uart_recv_flag = 1;  /* 标记接收完成 */
            break;  /* 每次中断只处理一帧 */
        }

    }
}

/* CDC transfers outlive this call, so responses need persistent storage. */
static int enqueue(const uint8_t *data, uint16_t length)
{
    uint16_t i;
    if (length > TX_SIZE - (uint16_t)(tx_head - tx_tail)) { return 0; }
    for (i = 0; i < length; ++i) { tx_ring[tx_head++ & (TX_SIZE - 1U)] = data[i]; }
    return 1;
}

int control_transport_write(const uint8_t *data, uint16_t length)
{
    int accepted = enqueue(data, length);
#if USB_DIAG_ENABLE
    if (accepted) { usb_diag.reply_bytes += length; }
    else { ++usb_diag.reply_failures; }
#endif
    return accepted;
}

/* Debug output cannot consume the last 32 bytes needed by an A4 reply. */
int control_transport_debug_write(const uint8_t *data, uint16_t length)
{
    if (length > TX_SIZE - (uint16_t)(tx_head - tx_tail) ||
        TX_SIZE - (uint16_t)(tx_head - tx_tail) - length < DEBUG_REPLY_RESERVE) {
#if USB_DIAG_ENABLE
        usb_diag.debug_drops += length;
#endif
        return 0;
    }
    return enqueue(data, length);
}

void control_transport_poll(void)
{
    uint32_t mask = __get_PRIMASK();
    USBD_CDC_HandleTypeDef *cdc;
    uint16_t length, i;
    __disable_irq();
    cdc = (USBD_CDC_HandleTypeDef *)hUsbDeviceFS.pClassData;
#if USB_DIAG_ENABLE
    if (cdc != NULL && cdc->TxState != 0 && hUsbDeviceFS.dev_state == USBD_STATE_CONFIGURED) {
        if (!busy_active) { busy_tick = HAL_GetTick(); busy_active = 1; }
        usb_diag.busy_ms = HAL_GetTick() - busy_tick;
        if (usb_diag.busy_ms > usb_diag.busy_max_ms) { usb_diag.busy_max_ms = usb_diag.busy_ms; }
    } else { busy_active = 0; usb_diag.busy_ms = 0; }
#endif
    if (hUsbDeviceFS.dev_state == USBD_STATE_CONFIGURED && cdc != NULL && cdc->TxState == 0) {
        length = (uint16_t)(tx_head - tx_tail);
        if (length > sizeof(tx_packet)) { length = sizeof(tx_packet); }
        if (length != 0) {
            for (i = 0; i < length; ++i) { tx_packet[i] = tx_ring[(tx_tail + i) & (TX_SIZE - 1U)]; }
            if (CDC_Transmit_FS(tx_packet, length) == USBD_OK) {
                tx_tail += length;
#if USB_DIAG_ENABLE
                ++usb_diag.tx_submits;
#endif
            }
        }
    }
    __set_PRIMASK(mask);
}

#if USB_DIAG_ENABLE
void usb_diag_stage(uint32_t phase)
{
    usb_diag.phase = phase;
    usb_diag.phase_since_ms = HAL_GetTick();
}

void usb_diag_poll(void)
{
    char line[224];
    uint8_t rx[8];
    uint32_t mask, now = HAL_GetTick();
    uint16_t i;
    int count;
    USBD_CDC_HandleTypeDef *cdc = (USBD_CDC_HandleTypeDef *)hUsbDeviceFS.pClassData;
    /* Only report when no application data is queued. Never print from IRQ. */
    if ((uint32_t)(now - report_tick) < 1000U || uart_recv_flag || tx_head != tx_tail ||
        hUsbDeviceFS.dev_state != USBD_STATE_CONFIGURED || cdc == NULL || cdc->TxState != 0) { return; }
    report_tick = now;
    if (report_line == 0) {
        mask = __get_PRIMASK();
        __disable_irq();
        for (i = 0; i < 8; ++i) { rx[i] = usb_diag.last_rx[i]; }
        __set_PRIMASK(mask);
        count = snprintf(line, sizeof(line),
            "[RX t=%lu] p=%lu b=%lu f=%lu d=%lu q=%lu ow=%lu bad=%lu n=%u data=%02X%02X%02X%02X%02X%02X%02X%02X\r\n",
            (unsigned long)now, (unsigned long)usb_diag.rx_packets, (unsigned long)usb_diag.rx_bytes,
            (unsigned long)usb_diag.frames, (unsigned long)usb_diag.dispatches, (unsigned long)usb_diag.queries,
            (unsigned long)usb_diag.overwrites, (unsigned long)usb_diag.bad_bytes, usb_diag.last_rx_length,
            rx[0],rx[1],rx[2],rx[3],rx[4],rx[5],rx[6],rx[7]);
    } else if (report_line == 1) {
        count = snprintf(line, sizeof(line),
            "[TX t=%lu] rsp=%lu fail=%lu submit=%lu busy=%lu/%lu drop=%lu init=%lu/%lu latency=%lu\r\n",
            (unsigned long)now, (unsigned long)usb_diag.reply_bytes, (unsigned long)usb_diag.reply_failures,
            (unsigned long)usb_diag.tx_submits, (unsigned long)usb_diag.busy_ms, (unsigned long)usb_diag.busy_max_ms,
            (unsigned long)usb_diag.debug_drops, (unsigned long)usb_diag.usb_inits, (unsigned long)usb_diag.usb_deinits,
            (unsigned long)usb_diag.latency_max_ms);
    } else {
        count = snprintf(line, sizeof(line),
            "[NET t=%lu] max=%lu/%lu/%lu/%lu wait=%lu out=%lu tx=%lu down=%lu to=%lu tcr=%02X spimax=%lu spierr=%lu hal=%u reg=%02X:%02X id=%04X:%04X nsr=%02X rst=%08lX\r\n",
            (unsigned long)now, (unsigned long)usb_diag.loop_max_ms, (unsigned long)usb_diag.irq_max_ms,
            (unsigned long)usb_diag.timer_max_ms, (unsigned long)usb_diag.command_max_ms,
            (unsigned long)usb_diag.net_tx_wait_max_ms, (unsigned long)usb_diag.net_tx_max_ms,
            (unsigned long)usb_diag.net_tx_calls, (unsigned long)usb_diag.net_tx_link_drops,
            (unsigned long)usb_diag.net_tx_timeouts, usb_diag.last_tcr, (unsigned long)usb_diag.spi_max_ms,
            (unsigned long)usb_diag.spi_errors, usb_diag.spi_status, usb_diag.last_reg, usb_diag.last_value,
            usb_diag.vendor, usb_diag.product, usb_diag.nsr, (unsigned long)usb_diag.reset_flags);
    }
    report_line = (uint8_t)((report_line + 1U) % 3U);
    if (count > 0 && count < (int)sizeof(line)) { (void)control_transport_debug_write((const uint8_t *)line, (uint16_t)count); }
}
#endif