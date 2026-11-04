#include "control_transport.h"
#include "main.h"
#include "usb_device.h"
#include "usbd_cdc_if.h"
#include "usbd_cdc.h"
#include <string.h>

extern USBD_HandleTypeDef hUsbDeviceFS;

#define RX_SIZE 256U
#define TX_SIZE 256U
typedef struct {
    uint8_t bytes[RX_SIZE];
    volatile uint16_t head;
    volatile uint16_t tail;
} RxRing;
static RxRing usb_rx;
static RxRing tcp_rx;
static uint8_t tx_ring[TX_SIZE];
static uint8_t tx_packet[64];
static uint16_t tx_head;
static uint16_t tx_tail;
uint8_t uart_recv_arr[8];
volatile uint8_t uart_recv_flag;
uint8_t tcp_recv_arr[8];
uint8_t tcp_recv_flag;
volatile uint32_t usb_rx_overruns;
volatile uint32_t usb_tx_drops;

void control_transport_usb_rx(const uint8_t *data, uint32_t length)
{
    uint32_t i;
    /* Producer is USB IRQ; consumer is the foreground. Drop a whole packet
       on overflow rather than overwriting a frame being consumed. */
    if (length > RX_SIZE - (uint16_t)(usb_rx.head - usb_rx.tail)) {
        ++usb_rx_overruns;
        return;
    }
    for (i = 0; i < length; ++i) {
        usb_rx.bytes[usb_rx.head & (RX_SIZE - 1U)] = data[i];
        __DMB();
        ++usb_rx.head;
    }
}

uint16_t control_transport_tcp_space(void)
{
    return RX_SIZE - (uint16_t)(tcp_rx.head - tcp_rx.tail);
}

int control_transport_tcp_rx(const uint8_t *data, uint16_t length)
{
    uint16_t i;
    if (length > RX_SIZE - (uint16_t)(tcp_rx.head - tcp_rx.tail)) { return 0; }
    for (i = 0; i < length; ++i) {
        tcp_rx.bytes[tcp_rx.head++ & (RX_SIZE - 1U)] = data[i];
    }
    return 1;
}

static int get_frame(RxRing *ring, uint8_t frame[8])
{
    while ((uint16_t)(ring->head - ring->tail) >= 8U) {
        unsigned i;
        uint16_t tail = ring->tail;
        if (ring->bytes[tail & (RX_SIZE - 1U)] != 0xFE ||
            ring->bytes[(tail + 7U) & (RX_SIZE - 1U)] != 0xFF) {
            ++ring->tail;
            continue;
        }
        for (i = 0; i < 8; ++i) { frame[i] = ring->bytes[(tail + i) & (RX_SIZE - 1U)]; }
        __DMB();
        ring->tail = tail + 8U;
        return 1;
    }
    return 0;
}

int control_transport_write(const uint8_t *data, uint16_t length)
{
    uint16_t i;
    if (length > TX_SIZE - (uint16_t)(tx_head - tx_tail)) {
        ++usb_tx_drops;
        return 0;
    }
    for (i = 0; i < length; ++i) { tx_ring[tx_head++ & (TX_SIZE - 1U)] = data[i]; }
    return 1;
}

void control_transport_poll(void)
{
    USBD_CDC_HandleTypeDef *cdc;
    uint16_t length;
    if (get_frame(&tcp_rx, tcp_recv_arr)) {
        tcp_recv_flag = 1;
        tcp_recv_data_process();
        tcp_recv_flag = 0;
    }
    if (get_frame(&usb_rx, uart_recv_arr)) {
        uart_recv_flag = 1;
        uart_recv_data_process();
        uart_recv_flag = 0;
    }
    cdc = (USBD_CDC_HandleTypeDef *)hUsbDeviceFS.pClassData;
    if (hUsbDeviceFS.dev_state != USBD_STATE_CONFIGURED || cdc == NULL || cdc->TxState != 0) { return; }
    length = (uint16_t)(tx_head - tx_tail);
    if (length > sizeof(tx_packet)) { length = sizeof(tx_packet); }
    if (length != 0) {
        uint16_t i;
        for (i = 0; i < length; ++i) { tx_packet[i] = tx_ring[(tx_tail + i) & (TX_SIZE - 1U)]; }
        /* tx_packet remains untouched until TxState returns to zero. */
        if (CDC_Transmit_FS(tx_packet, length) == USBD_OK) { tx_tail += length; }
    }
}
