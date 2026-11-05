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

volatile uint8_t network_hw_ready;
volatile uint8_t network_last_error;
volatile uint32_t network_hw_failures;
static uint32_t last_link_check, last_hardware_attempt;

static void hardware_failed(void)
{
    network_last_error = ch390_get_error();
    ++network_hw_failures;
    network_hw_ready = 0;
    ethernetif_stop(&ch390_netif);
}

static void hardware_try_start(void)
{
    last_hardware_attempt = HAL_GetTick();
    if (ethernetif_start(&ch390_netif) != ERR_OK) { hardware_failed(); return; }
    network_hw_ready = 1;
    network_last_error = CH390_ERROR_NONE;
    if (ch390_get_link_status()) { netif_set_link_up(&ch390_netif); }
    if (ch390_get_error() != CH390_ERROR_NONE) { hardware_failed(); }
    last_link_check = HAL_GetTick();
}

void network_app_init(void)
{
    ip4_addr_t ip, mask, gateway;
    FlashNetCfg_t config;
    HAL_Delay(10);
    if (flash_net_cfg_load(&config) == 0) {
        net_mode = 1;
        IP4_ADDR(&ip, config.ip[0], config.ip[1], config.ip[2], config.ip[3]);
        IP4_ADDR(&mask, config.mask[0], config.mask[1], config.mask[2], config.mask[3]);
        IP4_ADDR(&gateway, config.gw[0], config.gw[1], config.gw[2], config.gw[3]);
    } else {
        net_mode = 0;
        ip4_addr_set_zero(&ip);
        ip4_addr_set_zero(&mask);
        ip4_addr_set_zero(&gateway);
    }
    init_lwip_netif(&ip, &mask, &gateway);
    hardware_try_start();
    if (net_mode == 0 && dhcp_start(&ch390_netif) != ERR_OK) { Error_Handler(); }
    udp_discover_init();
    tcp_server_init();
    tcp_client_init();
}

void network_app_poll(void)
{
    uint32_t now = HAL_GetTick();
    unsigned budget;
    if (!network_hw_ready && (uint32_t)(now - last_hardware_attempt) >= 1000U) { hardware_try_start(); }
    if (network_hw_ready) {
        uint8_t status = 0;
        if (ch390_get_int_pin()) { status = ch390_get_int_status(); }
        if ((status & ISR_LNKCHG) != 0 || (uint32_t)(now - last_link_check) >= 250U) {
            last_link_check = now;
            if (ch390_get_link_status()) { netif_set_link_up(&ch390_netif); }
            else { netif_set_link_down(&ch390_netif); }
        }
        for (budget = 0; budget < 4 && ch390_get_error() == CH390_ERROR_NONE; ++budget) {
            struct ethernetif *state = (struct ethernetif *)ch390_netif.state;
            ethernetif_input(&ch390_netif);
            if (state->rx_len == 0) { break; }
        }
        if (ch390_get_error() != CH390_ERROR_NONE) { hardware_failed(); }
    }
    /* Timers and USB remain live when Ethernet hardware is missing or faulty. */
    sys_check_timeouts();
    control_transport_poll();
    tcp_server_poll();
}
