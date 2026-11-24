#include "network_app.h"
#include "main.h"
#include "CH390_Interface.h"
#include "control_transport.h"
#include "flash_net_cfg.h"
#include "netif/ethernetif.h"
#include "lwip/dhcp.h"
#include "lwip/timeouts.h"
#include "tcp_echo.h"
#include "udp_echo.h"
#include "usb_diag.h"
#include <stdio.h>

volatile uint8_t network_hw_ready, network_last_error;
volatile uint32_t network_hw_failures;
static uint32_t last_hardware_attempt, last_link_check;

static void hardware_failed(void)
{
    network_last_error = ch390_get_error();
    ++network_hw_failures;
    network_hw_ready = 0;
    ethernetif_stop(&ch390_netif);
}
static void update_link(void)
{
    if (ch390_get_link_status()) { netif_set_link_up(&ch390_netif); }
    else { netif_set_link_down(&ch390_netif); }
    last_link_check = HAL_GetTick();
}
static void try_hardware(void)
{
#if USB_DIAG_ENABLE
    uint32_t parent_phase = usb_diag.phase;
#endif
    USB_DIAG_STAGE(USB_DIAG_INIT);
    last_hardware_attempt = HAL_GetTick();
    if (ethernetif_start(&ch390_netif) != ERR_OK) { hardware_failed(); }
    else {
    network_hw_ready = 1;
    network_last_error = CH390_ERROR_NONE;
    update_link();
    if (ch390_get_error() != CH390_ERROR_NONE) { hardware_failed(); }
    }
#if USB_DIAG_ENABLE
    usb_diag_stage(parent_phase);
#endif
}
void network_app_init(void)
{
    ip4_addr_t ip, mask, gateway;
    FlashNetCfg_t config;
    USB_DIAG_STAGE(USB_DIAG_INIT);
#if USB_DIAG_ENABLE
    usb_diag.reset_flags = RCC->CSR;
#endif
    HAL_Delay(10);
    if (flash_net_cfg_load(&config) == 0 && config.net_mode == 1) {
        net_mode = 1;
        IP4_ADDR(&ip, config.ip[0], config.ip[1], config.ip[2], config.ip[3]);
        IP4_ADDR(&mask, config.mask[0], config.mask[1], config.mask[2], config.mask[3]);
        IP4_ADDR(&gateway, config.gw[0], config.gw[1], config.gw[2], config.gw[3]);
    } else {
        net_mode = 0;
        ip4_addr_set_zero(&ip); ip4_addr_set_zero(&mask); ip4_addr_set_zero(&gateway);
    }
    init_lwip_netif(&ip, &mask, &gateway);
    try_hardware();
    if (net_mode == 0) { (void)dhcp_start(&ch390_netif); }
    udp_discover_init();
    tcp_server_init();
    tcp_client_init();
    printf("Network mode=%u ready=%u error=%u IP=%s\r\n", net_mode, network_hw_ready,
           network_last_error, ip4addr_ntoa(netif_ip4_addr(&ch390_netif)));
}
void network_app_poll(void)
{
    uint32_t now = HAL_GetTick();
    unsigned budget;
    USB_DIAG_BEGIN(loop_start, USB_DIAG_COMMAND);
    USB_DIAG_BEGIN(command_start, USB_DIAG_COMMAND);
    control_transport_poll();
    USB_DIAG_END(command_max_ms, command_start);
    if (!network_hw_ready && (uint32_t)(now - last_hardware_attempt) >= 1000U) { try_hardware(); }
    USB_DIAG_BEGIN(irq_start, USB_DIAG_NET_IRQ);
    if (network_hw_ready) {
        uint8_t status = ch390_get_int_pin() ? ch390_get_int_status() : 0;
        if ((status & ISR_LNKCHG) || (uint32_t)(now - last_link_check) >= 250U) { update_link(); }
        /* A lost/cleared RX interrupt must not strand buffered packets. */
        for (budget = 0; budget < 4U && ch390_get_error() == CH390_ERROR_NONE; ++budget) {
            struct ethernetif *state = (struct ethernetif *)ch390_netif.state;
            ethernetif_input(&ch390_netif);
            if (state->rx_len == 0) { break; }
        }
        if (ch390_get_error() != CH390_ERROR_NONE) { hardware_failed(); }
    }
    USB_DIAG_END(irq_max_ms, irq_start);
    USB_DIAG_BEGIN(timer_start, USB_DIAG_TIMERS);
    sys_check_timeouts();
    tcp_app_poll();
    USB_DIAG_END(timer_max_ms, timer_start);
    USB_DIAG_STAGE(USB_DIAG_TX);
    control_transport_poll();
    USB_DIAG_END(loop_max_ms, loop_start);
    USB_DIAG_STAGE(USB_DIAG_REPORT);
#if USB_DIAG_ENABLE
    usb_diag_poll();
#endif
    USB_DIAG_STAGE(USB_DIAG_IDLE);
}