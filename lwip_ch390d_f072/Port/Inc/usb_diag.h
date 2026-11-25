#ifndef USB_DIAG_H
#define USB_DIAG_H
#include <stdint.h>
#include "retarget.h"

/* Diagnostic counters are also visible in the SWD debugger. */
#ifndef USB_DIAG_ENABLE
#define USB_DIAG_ENABLE USB_PRINTF_ENABLE
#endif
#if USB_DIAG_ENABLE
enum {
    USB_DIAG_IDLE, USB_DIAG_INIT, USB_DIAG_NET_IRQ, USB_DIAG_TIMERS,
    USB_DIAG_COMMAND, USB_DIAG_TX, USB_DIAG_REPORT, USB_DIAG_NET_TX_WAIT,
    USB_DIAG_NET_OUTPUT
};
typedef struct {
    uint32_t rx_packets, rx_bytes, frames, overwrites, bad_bytes;
    uint32_t dispatches, queries, reply_bytes, reply_failures, tx_submits;
    uint32_t busy_ms, busy_max_ms, debug_drops, usb_inits, usb_deinits;
    uint32_t last_rx_ms, last_frame_ms, latency_max_ms;
    uint32_t loop_max_ms, irq_max_ms, timer_max_ms, command_max_ms;
    uint32_t spi_errors, phase, phase_since_ms, reset_flags;
    uint32_t net_tx_wait_max_ms, net_tx_max_ms, net_tx_calls, spi_max_ms;
    uint32_t net_tx_link_drops, net_tx_timeouts;
    uint16_t last_rx_length, vendor, product;
    uint8_t last_rx[8], last_command, last_reg, last_value, spi_status, nsr, last_tcr;
} UsbDiagnostics;
extern volatile UsbDiagnostics usb_diag;
void usb_diag_stage(uint32_t phase);
void usb_diag_poll(void);
#define USB_DIAG_STAGE(p) usb_diag_stage(p)
#define USB_DIAG_BEGIN(name, p) uint32_t name = HAL_GetTick(); USB_DIAG_STAGE(p)
#define USB_DIAG_END(field, name) do { \
    uint32_t elapsed_ = HAL_GetTick() - (name); \
    if (elapsed_ > usb_diag.field) { usb_diag.field = elapsed_; } \
} while (0)
#else
#define USB_DIAG_STAGE(p) ((void)0)
#define USB_DIAG_BEGIN(name, p) ((void)0)
#define USB_DIAG_END(field, name) ((void)0)
#endif
#endif

