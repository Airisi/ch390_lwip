/* F072 NO_SYS adapter; the shared G070 ethernetif is intentionally unchanged. */
#include "main.h"
#include "CH390_Interface.h"
#include "netif/ethernetif.h"
#include "lwip/init.h"
#include "lwip/etharp.h"
#include "lwip/pbuf.h"
#include "netif/ethernet.h"

struct netif ch390_netif;
static struct ethernetif ethernet_state;

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
    /* Never replace the TX SRAM while the preceding frame is transmitting. */
    while ((ch390_read_reg(CH390_TCR) & TCR_TXREQ) != 0) {
        if ((uint32_t)(HAL_GetTick() - start) >= 100U) { return ERR_TIMEOUT; }
    }
    for (part = packet; part != NULL; part = part->next) {
        ch390_write_mem(part->payload, part->len);
    }
    ch390_write_reg(CH390_TXPLL, (uint8_t)packet->tot_len);
    ch390_write_reg(CH390_TXPLH, (uint8_t)(packet->tot_len >> 8));
    ch390_send_request();
    return ERR_OK;
}

void ethernetif_input(struct netif *netif)
{
    struct pbuf *packet, *part;
    uint8_t ready, header[4];
    uint16_t wire_length;
    ethernet_state.rx_len = 0;
    (void)ch390_read_reg(CH390_MRCMDX);
    ready = ch390_read_reg(CH390_MRCMDX);
    if ((ready & CH390_PKT_ERR) != 0) { reset_rx_fifo(); return; }
    if ((ready & CH390_PKT_RDY) == 0) { return; }
    ch390_read_mem(header, sizeof(header));
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
    ch390_default_config();
    ch390_int_pin_config(INCR_TYPE_PP, INCR_POL_H);
    ch390_get_mac(netif->hwaddr);
    return ERR_OK;
}

void init_lwip_netif(const ip4_addr_t *ipaddr, const ip4_addr_t *netmask, const ip4_addr_t *gw)
{
    lwip_init();
    if (netif_add(&ch390_netif, ipaddr, netmask, gw, NULL, ethernetif_init, ethernet_input) == NULL) {
        Error_Handler();
    }
    netif_set_default(&ch390_netif);
    if (ch390_get_link_status()) { netif_set_link_up(&ch390_netif); }
    netif_set_up(&ch390_netif);
}

u32_t sys_now(void) { return HAL_GetTick(); }
