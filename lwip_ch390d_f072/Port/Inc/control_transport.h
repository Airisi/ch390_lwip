#ifndef CONTROL_TRANSPORT_H
#define CONTROL_TRANSPORT_H
#include <stdint.h>
extern uint8_t uart_recv_arr[8];
extern volatile uint8_t uart_recv_flag;
extern uint8_t tcp_recv_arr[8];
extern uint8_t tcp_recv_flag;
extern volatile uint8_t net_mode;
void control_transport_usb_rx(const uint8_t *data, uint32_t length);
void control_transport_poll(void);
int control_transport_write(const uint8_t *data, uint16_t length);
int control_transport_debug_write(const uint8_t *data, uint16_t length);
void tcp_recv_data_process(void);
void uart_recv_data_process(void);
void send_net_mode_ack(void);
void query_net_config(void);
#endif
