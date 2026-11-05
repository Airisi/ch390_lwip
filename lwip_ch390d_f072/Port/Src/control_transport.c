#include "control_transport.h"
#include "main.h"
#include "usb_device.h"
#include "usbd_cdc_if.h"
#include "usbd_cdc.h"

extern USBD_HandleTypeDef hUsbDeviceFS;
#define RX_SIZE 256U
#define TX_SIZE 256U
/* The largest built-in reply is the four-frame configuration query. */
#define USB_REPLY_RESERVE 32U
typedef struct {
    uint8_t bytes[RX_SIZE];
    volatile uint16_t head;
    volatile uint16_t tail;
} RxRing;
static RxRing usb_rx, tcp_rx;
static uint8_t tx_ring[TX_SIZE], tx_packet[64];
static uint16_t tx_head, tx_tail;
static volatile uint32_t usb_generation;
static uint32_t dispatch_generation;
static uint8_t usb_dispatch_active;
uint8_t uart_recv_arr[8], tcp_recv_arr[8];
volatile uint8_t uart_recv_flag;
uint8_t tcp_recv_flag;
/* RX counts deferrals, not dropped packets: the CDC endpoint retains them. */
volatile uint32_t usb_rx_overruns, usb_tx_drops;

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
    uint16_t space = RX_SIZE - (uint16_t)(usb_rx.head - usb_rx.tail);
    irq_restore(mask);
    return space;
}

void control_transport_usb_reset(void)
{
    uint32_t mask = irq_save();
    ++usb_generation;
    usb_rx.head = usb_rx.tail = 0;
    tx_head = tx_tail = 0;
    uart_recv_flag = 0;
    /* Do not change the token of an already executing command. Its replies
       must be rejected if a USB IRQ has ended that command's session. */
    irq_restore(mask);
}

int control_transport_usb_rx(const uint8_t *data, uint32_t length)
{
    uint32_t mask = irq_save(), i;
    uint16_t head = usb_rx.head;
    if (length > RX_SIZE - (uint16_t)(head - usb_rx.tail)) {
        ++usb_rx_overruns;
        irq_restore(mask);
        return 0;
    }
    for (i = 0; i < length; ++i) { usb_rx.bytes[(head + i) & (RX_SIZE - 1U)] = data[i]; }
    __DMB();
    usb_rx.head = head + (uint16_t)length;
    irq_restore(mask);
    return 1;
}

uint16_t control_transport_tcp_space(void)
{
    return RX_SIZE - (uint16_t)(tcp_rx.head - tcp_rx.tail);
}
int control_transport_tcp_idle(void) { return tcp_rx.head == tcp_rx.tail && !tcp_recv_flag; }
void control_transport_tcp_reset(void) { tcp_rx.head = tcp_rx.tail = 0; }
int control_transport_tcp_rx(const uint8_t *data, uint16_t length)
{
    uint16_t i;
    if (length > control_transport_tcp_space()) { return 0; }
    for (i = 0; i < length; ++i) { tcp_rx.bytes[tcp_rx.head++ & (RX_SIZE - 1U)] = data[i]; }
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
    uint32_t mask = irq_save();
    uint16_t i;
    if ((usb_dispatch_active && dispatch_generation != usb_generation) ||
        length > TX_SIZE - (uint16_t)(tx_head - tx_tail)) {
        ++usb_tx_drops;
        irq_restore(mask);
        return 0;
    }
    for (i = 0; i < length; ++i) { tx_ring[tx_head++ & (TX_SIZE - 1U)] = data[i]; }
    irq_restore(mask);
    return 1;
}

void control_transport_poll(void)
{
    uint32_t mask;
    int usb_frame = 0;
    USBD_CDC_HandleTypeDef *cdc;
    uint16_t length;
    if (get_frame(&tcp_rx, tcp_recv_arr)) {
        tcp_recv_flag = 1;
        tcp_recv_data_process();
        tcp_recv_flag = 0;
    }
    mask = irq_save();
    if (TX_SIZE - (uint16_t)(tx_head - tx_tail) >= USB_REPLY_RESERVE) {
        usb_frame = get_frame(&usb_rx, uart_recv_arr);
        if (usb_frame) {
            dispatch_generation = usb_generation;
            usb_dispatch_active = 1;
        }
    }
    irq_restore(mask);
    if (usb_frame) {
        uart_recv_flag = 1;
        uart_recv_data_process();
        uart_recv_flag = 0;
        usb_dispatch_active = 0;
    }
    CDC_Receive_Resume_FS();
    mask = irq_save();
    cdc = (USBD_CDC_HandleTypeDef *)hUsbDeviceFS.pClassData;
    if (hUsbDeviceFS.dev_state == USBD_STATE_CONFIGURED && cdc != NULL && cdc->TxState == 0) {
        length = (uint16_t)(tx_head - tx_tail);
        if (length > sizeof(tx_packet)) { length = sizeof(tx_packet); }
        if (length != 0) {
            uint16_t i;
            for (i = 0; i < length; ++i) { tx_packet[i] = tx_ring[(tx_tail + i) & (TX_SIZE - 1U)]; }
            /* Copy, submit and commit the queue in the same USB session. */
            if (CDC_Transmit_FS(tx_packet, length) == USBD_OK) { tx_tail += length; }
        }
    }
    irq_restore(mask);
}
