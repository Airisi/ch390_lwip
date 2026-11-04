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

static uint32_t last_link_check;

void network_app_init(void)
{
    ip4_addr_t ip, mask, gateway;
    FlashNetCfg_t config;
    HAL_Delay(10);
    ch390_software_reset();
    HAL_Delay(2);
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
    if (net_mode == 0 && dhcp_start(&ch390_netif) != ERR_OK) { Error_Handler(); }
    udp_discover_init();
    tcp_server_init();
    tcp_client_init();
}

void network_app_poll(void)
{
    uint32_t now = HAL_GetTick();
    uint8_t status = 0;
    unsigned budget;
    if (ch390_get_int_pin()) { status = ch390_get_int_status(); }
    if ((status & ISR_LNKCHG) != 0 || (uint32_t)(now - last_link_check) >= 250U) {
        last_link_check = now;
        if (ch390_get_link_status()) { netif_set_link_up(&ch390_netif); }
        else { netif_set_link_down(&ch390_netif); }
    }
    /* Poll RX even without a new IRQ edge. A budget prevents an RX flood from
       starving CDC commands, DHCP/TCP timeouts and the foreground loop. */
    for (budget = 0; budget < 4; ++budget) {
        struct ethernetif *state = (struct ethernetif *)ch390_netif.state;
        ethernetif_input(&ch390_netif);
        if (state->rx_len == 0) { break; }
    }
    sys_check_timeouts();
    control_transport_poll();
}
