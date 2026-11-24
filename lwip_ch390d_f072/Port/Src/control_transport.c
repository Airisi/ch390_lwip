/* USB bytes are queued by IRQ; parsing and command execution are foreground. */
#include "control_transport.h"
#include "main.h"
#include "usb_device.h"
#include "usbd_cdc_if.h"
#include "usbd_cdc.h"
#include "usb_diag.h"
#include <stdio.h>
#include <string.h>

extern USBD_HandleTypeDef hUsbDeviceFS;
#define RX_SIZE 256U
#define TX_SIZE 256U
#define DEBUG_REPLY_RESERVE 32U
static uint8_t rx_ring[RX_SIZE], usb_frame[8];
static volatile uint8_t frame_used;
static volatile uint16_t rx_head, rx_tail, tx_head, tx_tail;
static uint8_t tx_ring[TX_SIZE], tx_packet[64];
static volatile uint32_t usb_generation;
static uint32_t dispatch_generation;
static uint8_t dispatch_active;
uint8_t uart_recv_arr[8];
volatile uint8_t uart_recv_flag;
#if USB_DIAG_ENABLE
volatile UsbDiagnostics usb_diag;
static uint32_t report_tick, busy_tick;
static uint8_t report_line, busy_active;
#endif
static uint32_t irq_save(void)
{
    uint32_t mask = __get_PRIMASK();
    __disable_irq();
    return mask;
}
static void irq_restore(uint32_t mask) { __set_PRIMASK(mask); }
uint16_t control_transport_usb_space(void)
{
    uint32_t mask = irq_save();
    uint16_t result = RX_SIZE - (uint16_t)(rx_head - rx_tail);
    irq_restore(mask);
    return result;
}
void control_transport_usb_reset(void)
{
    uint32_t mask = irq_save();
    ++usb_generation;
    rx_head = rx_tail = tx_head = tx_tail = 0;
    frame_used = 0;
    uart_recv_flag = 0;
    /* Leave an executing command's generation intact to reject stale replies. */
    irq_restore(mask);
}
int control_transport_usb_rx(const uint8_t *data, uint32_t length)
{
    uint32_t mask = irq_save(), i;
    uint16_t head = rx_head;
    if (length > RX_SIZE - (uint16_t)(head - rx_tail)) { irq_restore(mask); return 0; }
    for (i = 0; i < length; ++i) { rx_ring[(head + i) & (RX_SIZE - 1U)] = data[i]; }
    __DMB();
    rx_head = head + (uint16_t)length;
#if USB_DIAG_ENABLE
    ++usb_diag.rx_packets;
    usb_diag.rx_bytes += length;
    usb_diag.last_rx_ms = HAL_GetTick();
    usb_diag.last_rx_length = (uint16_t)length;
    for (i = 0; i < 8; ++i) { usb_diag.last_rx[i] = i < length ? data[i] : 0; }
#endif
    irq_restore(mask);
    return 1;
}
/* Called with IRQs masked, so reset cannot split parsing and dispatch capture. */
static int get_usb_frame(void)
{
    while (rx_tail != rx_head) {
        uint8_t byte = rx_ring[rx_tail++ & (RX_SIZE - 1U)];
        if (frame_used == 0 && byte != 0xFE) {
#if USB_DIAG_ENABLE
            ++usb_diag.bad_bytes;
#endif
            continue;
        }
        usb_frame[frame_used++] = byte;
        if (frame_used == 8U) {
            if (usb_frame[7] == 0xFF) {
                memcpy(uart_recv_arr, usb_frame, 8);
                frame_used = 0;
#if USB_DIAG_ENABLE
                ++usb_diag.frames;
                usb_diag.last_frame_ms = HAL_GetTick();
#endif
                return 1;
            } else {
                uint8_t next = 1;
                while (next < 8U && usb_frame[next] != 0xFE) { ++next; }
                frame_used = (uint8_t)(8U - next);
                memmove(usb_frame, usb_frame + next, frame_used);
#if USB_DIAG_ENABLE
                usb_diag.bad_bytes += next;
#endif
            }
        }
    }
    return 0;
}
static int enqueue(const uint8_t *data, uint16_t length, uint16_t reserve)
{
    uint16_t i, free = TX_SIZE - (uint16_t)(tx_head - tx_tail);
    if ((dispatch_active && dispatch_generation != usb_generation) ||
        length > free || free - length < reserve) { return 0; }
    for (i = 0; i < length; ++i) { tx_ring[tx_head++ & (TX_SIZE - 1U)] = data[i]; }
    return 1;
}
int control_transport_write(const uint8_t *data, uint16_t length)
{
    uint32_t mask = irq_save();
    int accepted = enqueue(data, length, 0);
#if USB_DIAG_ENABLE
    if (accepted) { usb_diag.reply_bytes += length; }
    else { ++usb_diag.reply_failures; }
#endif
    irq_restore(mask);
    return accepted;
}
int control_transport_debug_write(const uint8_t *data, uint16_t length)
{
    uint32_t mask = irq_save();
    int accepted = enqueue(data, length, DEBUG_REPLY_RESERVE);
#if USB_DIAG_ENABLE
    if (!accepted) { usb_diag.debug_drops += length; }
#endif
    irq_restore(mask);
    return accepted;
}
static void flush_tx(void)
{
    uint32_t mask = irq_save();
    uint16_t length, i;
    USBD_CDC_HandleTypeDef *cdc = (USBD_CDC_HandleTypeDef *)hUsbDeviceFS.pClassData;
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
    irq_restore(mask);
}
void control_transport_poll(void)
{
    uint32_t mask;
    int frame = 0;
    USB_DIAG_STAGE(USB_DIAG_TX);
    flush_tx();
    mask = irq_save();
    if (TX_SIZE - (uint16_t)(tx_head - tx_tail) >= DEBUG_REPLY_RESERVE) {
        frame = get_usb_frame();
        if (frame) {
            dispatch_generation = usb_generation;
            dispatch_active = 1;
            uart_recv_flag = 1;
#if USB_DIAG_ENABLE
            uint32_t latency = HAL_GetTick() - usb_diag.last_rx_ms;
            ++usb_diag.dispatches;
            usb_diag.last_command = uart_recv_arr[1];
            if (uart_recv_arr[1] == 0xA4) { ++usb_diag.queries; }
            if (latency > usb_diag.latency_max_ms) { usb_diag.latency_max_ms = latency; }
#endif
        }
    }
    irq_restore(mask);
    if (frame) {
        USB_DIAG_BEGIN(command_time, USB_DIAG_COMMAND);
        if (dispatch_generation == usb_generation) { uart_recv_data_process(); }
        uart_recv_flag = 0;
        dispatch_active = 0;
        USB_DIAG_END(command_max_ms, command_time);
    }
    USB_DIAG_STAGE(USB_DIAG_TX);
    CDC_Receive_Resume_FS();
    flush_tx();
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
    USBD_CDC_HandleTypeDef *cdc;
    /* Snapshot class readiness while reset/deconfiguration cannot free it. */
    mask = irq_save();
    cdc = (USBD_CDC_HandleTypeDef *)hUsbDeviceFS.pClassData;
    if ((uint32_t)(now - report_tick) < 1000U || uart_recv_flag || tx_head != tx_tail ||
        hUsbDeviceFS.dev_state != USBD_STATE_CONFIGURED || cdc == NULL || cdc->TxState != 0) {
        irq_restore(mask);
        return;
    }
    report_tick = now;
    irq_restore(mask);
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