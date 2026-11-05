/* Original application initialization and foreground ordering, on F072. */
#include "network_app.h"
#include "main.h"
#include "CH390_Interface.h"
#include "control_transport.h"
#include "flash_net_cfg.h"
#include "netif/ethernetif.h"
#include "lwip/dhcp.h"
#include "lwip/prot/dhcp.h"
#include "lwip/timeouts.h"
#include "tcp_echo.h"
#include "udp_echo.h"
#include <stdio.h>
#include <string.h>

static uint8_t dhcp_prev_bound;

static void ch390_int_handler(void)
{
    uint8_t int_status = ch390_get_int_status();
    struct ethernetif *state = ch390_netif.state;
    if (int_status & ISR_LNKCHG) {
        HAL_Delay(65);
        if (ch390_get_link_status()) { netif_set_link_up(&ch390_netif); }
        else { netif_set_link_down(&ch390_netif); }
        ch390_write_reg(CH390_ISR, ISR_LNKCHG);
    }
    if (int_status & ISR_ROS) {
        do { ethernetif_input(&ch390_netif); } while (state->rx_len != 0);
    }
    if (int_status & ISR_ROO) { printf("Overflow counter overflow\r\n"); }
    if (int_status & ISR_PR) {
        do { ethernetif_input(&ch390_netif); } while (state->rx_len != 0);
    }
}

void network_app_init(void)
{
    ip4_addr_t ip, mask, gateway;
    FlashNetCfg_t config;
    HAL_Delay(10);
    if (flash_net_cfg_load(&config) == 0 && config.net_mode == 1) {
        net_mode = 1;
        IP4_ADDR(&ip, config.ip[0], config.ip[1], config.ip[2], config.ip[3]);
        IP4_ADDR(&mask, config.mask[0], config.mask[1], config.mask[2], config.mask[3]);
        IP4_ADDR(&gateway, config.gw[0], config.gw[1], config.gw[2], config.gw[3]);
        init_lwip_netif(&ip, &mask, &gateway);
        printf("Load static IP from flash\r\n");
        printf("IP  : %s\r\n", ip4addr_ntoa(&ip));
        printf("Mask: %s\r\n", ip4addr_ntoa(&mask));
        printf("GW  : %s\r\n", ip4addr_ntoa(&gateway));
    } else {
        printf("DHCP mode\r\n");
        net_mode = 0;
        IP4_ADDR(&ip, 0, 0, 0, 0);
        IP4_ADDR(&mask, 0, 0, 0, 0);
        IP4_ADDR(&gateway, 0, 0, 0, 0);
        init_lwip_netif(&ip, &mask, &gateway);
    }
    if (net_mode == 0) { dhcp_start(&ch390_netif); }
    udp_discover_init();
    tcp_client_init();
    tcp_server_init();
}

void network_app_poll(void)
{
    struct dhcp *dhcp;
    if (ch390_get_int_pin()) {
        ch390_int_handler();
        if (tcp_recv_arr[0] != 0 && tcp_recv_flag == 1) {
            tcp_recv_flag = 0;
            tcp_recv_data_process();
            memset(tcp_recv_arr, 0, sizeof(tcp_recv_arr));
        }
    }
    sys_check_timeouts();
    if (uart_recv_flag == 1) {
        uart_recv_flag = 0;
        uart_recv_data_process();
    }
    /* USB IN is asynchronous; keep the submission buffer alive until complete. */
    control_transport_poll();
    if (net_mode == 0) {
        dhcp = netif_dhcp_data(&ch390_netif);
        if (dhcp != NULL && dhcp->state != DHCP_STATE_BOUND) {
            if (ch390_get_int_pin()) { ch390_int_handler(); }
            sys_check_timeouts();
            dhcp_prev_bound = 0;
        } else if (dhcp != NULL && dhcp->state == DHCP_STATE_BOUND && dhcp_prev_bound == 0) {
            dhcp_prev_bound = 1;
            printf("DHCP OK!\r\n");
            printf("IP  : %s\r\n", ip4addr_ntoa(netif_ip4_addr(&ch390_netif)));
            printf("Mask: %s\r\n", ip4addr_ntoa(netif_ip4_netmask(&ch390_netif)));
            printf("GW  : %s\r\n", ip4addr_ntoa(netif_ip4_gw(&ch390_netif)));
        }
    }
}
