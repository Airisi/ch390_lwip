/* F072 NO_SYS Ethernet adapter with recoverable CH390 initialization. */
#include "main.h"
#include "CH390_Interface.h"
#include "netif/ethernetif.h"
#include "lwip/init.h"
#include "lwip/etharp.h"
#include "lwip/pbuf.h"
#include "netif/ethernet.h"
#include <string.h>

struct netif ch390_netif;
static struct ethernetif ethernet_state;
static uint8_t hardware_ready;

static void reset_rx_fifo(void)
{
    ch390_write_reg(CH390_RCR, 0);
    ch390_write_reg(CH390_MPTRCR, 0x01);
    ch390_write_reg(CH390_MRRH, 0x0c);
    HAL_Delay(1);
    ch390_write_reg(CH390_RCR, RCR_RXEN | RCR_DIS_CRC);
    ethernet_state.rx_len = 0;
}

static err_t output(struct netif *netif, struct pbuf *packet)
{
    struct pbuf *part;
    uint32_t start = HAL_GetTick();
    (void)netif;
    if (!hardware_ready || ch390_get_error() != CH390_ERROR_NONE) { return ERR_IF; }
    /* Never replace the TX SRAM while the preceding frame is transmitting. */
    while ((ch390_read_reg(CH390_TCR) & TCR_TXREQ) != 0) {
        if (ch390_get_error() != CH390_ERROR_NONE) { return ERR_IF; }
        if ((uint32_t)(HAL_GetTick() - start) >= 100U) {
            ch390_set_error(CH390_ERROR_BUSY);
            return ERR_TIMEOUT;
        }
    }
    for (part = packet; part != NULL; part = part->next) {
        ch390_write_mem(part->payload, part->len);
    }
    ch390_write_reg(CH390_TXPLL, (uint8_t)packet->tot_len);
    ch390_write_reg(CH390_TXPLH, (uint8_t)(packet->tot_len >> 8));
    ch390_send_request();
    return ch390_get_error() == CH390_ERROR_NONE ? ERR_OK : ERR_IF;
}

void ethernetif_input(struct netif *netif)
{
    struct pbuf *packet, *part;
    uint8_t ready, header[4];
    uint16_t wire_length;
    ethernet_state.rx_len = 0;
    if (!hardware_ready || ch390_get_error() != CH390_ERROR_NONE) { return; }
    (void)ch390_read_reg(CH390_MRCMDX);
    ready = ch390_read_reg(CH390_MRCMDX);
    if ((ready & CH390_PKT_ERR) != 0) { reset_rx_fifo(); return; }
    if ((ready & CH390_PKT_RDY) == 0) { return; }
    ch390_read_mem(header, sizeof(header));
    if (ch390_get_error() != CH390_ERROR_NONE) { return; }
    wire_length = (uint16_t)header[2] | ((uint16_t)header[3] << 8);
    if (wire_length < 18U || wire_length > CH390_PKT_MAX) {
        reset_rx_fifo();
        return;
    }
    ethernet_state.rx_len = wire_length - 4U;
    ethernet_state.rx_status = header[1];
    if ((header[1] & 0x3fU) != 0) { ch390_drop_packet(wire_length); return; }
    packet = pbuf_alloc(PBUF_RAW, ethernet_state.rx_len, PBUF_POOL);
    if (packet == NULL) { ch390_drop_packet(wire_length); return; }
    for (part = packet; part != NULL; part = part->next) {
        ch390_read_mem(part->payload, part->len);
    }
    ch390_drop_packet(4); /* CRC is not passed to lwIP. */
    if (ch390_get_error() != CH390_ERROR_NONE) { pbuf_free(packet); return; }
    if (netif->input(packet, netif) != ERR_OK) { pbuf_free(packet); }
}

err_t ethernetif_init(struct netif *netif)
{
    netif->state = &ethernet_state;
    netif->name[0] = 'e';
    netif->name[1] = 'n';
    netif->hwaddr_len = ETH_HWADDR_LEN;
    netif->mtu = 1500;
    netif->flags = NETIF_FLAG_BROADCAST | NETIF_FLAG_ETHARP;
    netif->output = etharp_output;
    netif->linkoutput = output;
    memset(netif->hwaddr, 0, ETH_HWADDR_LEN);
    return ERR_OK;
}

void ethernetif_stop(struct netif *netif)
{
    hardware_ready = 0;
    ethernet_state.rx_len = 0;
    netif_set_link_down(netif);
}

err_t ethernetif_start(struct netif *netif)
{
    uint16_t vendor, product;
    uint8_t mac[ETH_HWADDR_LEN], any = 0;
    unsigned i;
    hardware_ready = 0;
    ch390_clear_error();
    ch390_software_reset();
    if (ch390_get_error() != CH390_ERROR_NONE) { return ERR_IF; }
    HAL_Delay(2);
    vendor = ch390_get_vendor_id();
    product = ch390_get_product_id();
    if (ch390_get_error() != CH390_ERROR_NONE) { return ERR_IF; }
    if (vendor == 0 || vendor == 0xFFFF || product == 0 || product == 0xFFFF) {
        ch390_set_error(CH390_ERROR_ID);
        return ERR_IF;
    }
    ch390_default_config();
    if (ch390_get_error() != CH390_ERROR_NONE) { return ERR_IF; }
    ch390_int_pin_config(INCR_TYPE_PP, INCR_POL_H);
    ch390_get_mac(mac);
    if (ch390_get_error() != CH390_ERROR_NONE) { return ERR_IF; }
    for (i = 0; i < sizeof(mac); ++i) { any |= mac[i]; }
    if (any == 0 || (mac[0] & 1U) != 0) {
        ch390_set_error(CH390_ERROR_MAC);
        return ERR_IF;
    }
    memcpy(netif->hwaddr, mac, sizeof(mac));
    hardware_ready = 1;
    return ERR_OK;
}

void init_lwip_netif(const ip4_addr_t *ipaddr, const ip4_addr_t *netmask, const ip4_addr_t *gw)
{
    lwip_init();
    if (netif_add(&ch390_netif, ipaddr, netmask, gw, NULL, ethernetif_init, ethernet_input) == NULL) {
        Error_Handler();
    }
    netif_set_default(&ch390_netif);
    netif_set_link_down(&ch390_netif);
    netif_set_up(&ch390_netif);
}

u32_t sys_now(void) { return HAL_GetTick(); }
