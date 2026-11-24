/**
 * @file
 * Ethernet Interface Skeleton
 *
 */

/*
 * Copyright (c) 2001-2004 Swedish Institute of Computer Science.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
 * SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
 * OF SUCH DAMAGE.
 *
 * This file is part of the lwIP TCP/IP stack.
 *
 * Author: Adam Dunkels <adam@sics.se>
 *
 */

/*
 * This file is a skeleton for developing Ethernet network interface
 * drivers for lwIP. Add code to the low_level functions and do a
 * search-and-replace for the word "ethernetif" to replace it with
 * something that better describes your network interface.
 */

#include "main.h"
#include "CH390_Interface.h"
#include "usb_diag.h"
#include "lwip/init.h"
#include "lwip/etharp.h"
#include "lwip/pbuf.h"
#include "netif/ethernet.h"
#include "netif/ethernetif.h"
#include <string.h>

#define CH390_TX_WAIT_MS 100U
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
static err_t low_level_output(struct netif *netif, struct pbuf *packet)
{
    struct pbuf *part;
    uint8_t value;
    uint32_t wait_start;
    err_t result = ERR_OK;
#if USB_DIAG_ENABLE
    uint32_t parent_phase = usb_diag.phase, output_start = HAL_GetTick();
    ++usb_diag.net_tx_calls;
#endif
    (void)netif;
    USB_DIAG_STAGE(USB_DIAG_NET_OUTPUT);
    if (!hardware_ready || ch390_get_error() != CH390_ERROR_NONE) { result = ERR_IF; goto done; }
    value = ch390_read_reg(CH390_NSR);
    if (ch390_get_error() != CH390_ERROR_NONE) { result = ERR_IF; goto done; }
    if ((value & NSR_LINKST) == 0) {
#if USB_DIAG_ENABLE
        ++usb_diag.net_tx_link_drops;
#endif
        result = ERR_IF;
        goto done;
    }
    wait_start = HAL_GetTick();
    USB_DIAG_STAGE(USB_DIAG_NET_TX_WAIT);
    for (;;) {
        value = ch390_read_reg(CH390_TCR);
#if USB_DIAG_ENABLE
        usb_diag.last_tcr = value;
#endif
        if (ch390_get_error() != CH390_ERROR_NONE) { result = ERR_IF; break; }
        if ((value & TCR_TXREQ) == 0) { break; }
        if ((uint32_t)(HAL_GetTick() - wait_start) >= CH390_TX_WAIT_MS) {
#if USB_DIAG_ENABLE
            ++usb_diag.net_tx_timeouts;
#endif
            ch390_set_error(CH390_ERROR_BUSY);
            result = ERR_TIMEOUT;
            break;
        }
        value = ch390_read_reg(CH390_NSR);
        if (ch390_get_error() != CH390_ERROR_NONE) { result = ERR_IF; break; }
        if ((value & NSR_LINKST) == 0) {
#if USB_DIAG_ENABLE
            ++usb_diag.net_tx_link_drops;
#endif
            result = ERR_IF;
            break;
        }
    }
    USB_DIAG_END(net_tx_wait_max_ms, wait_start);
    if (result != ERR_OK) { goto done; }
    USB_DIAG_STAGE(USB_DIAG_NET_OUTPUT);
    for (part = packet; part != NULL; part = part->next) { ch390_write_mem(part->payload, part->len); }
    if (ch390_get_error() != CH390_ERROR_NONE) { result = ERR_IF; goto done; }
    ch390_write_reg(CH390_TXPLL, (uint8_t)packet->tot_len);
    ch390_write_reg(CH390_TXPLH, (uint8_t)(packet->tot_len >> 8));
    ch390_send_request();
    if (ch390_get_error() != CH390_ERROR_NONE) { result = ERR_IF; }
done:
    USB_DIAG_END(net_tx_max_ms, output_start);
#if USB_DIAG_ENABLE
    usb_diag_stage(parent_phase);
#endif
    return result;
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
    if (ch390_get_error() != CH390_ERROR_NONE) { return; }
    if (ready & CH390_PKT_ERR) { reset_rx_fifo(); return; }
    if (!(ready & CH390_PKT_RDY)) { return; }
    ch390_read_mem(header, sizeof(header));
    if (ch390_get_error() != CH390_ERROR_NONE) { return; }
    wire_length = (uint16_t)header[2] | ((uint16_t)header[3] << 8);
    if (wire_length < 18U || wire_length > CH390_PKT_MAX) { reset_rx_fifo(); return; }
    ethernet_state.rx_len = wire_length - 4U;
    ethernet_state.rx_status = header[1];
    if (header[1] & 0x3FU) { ch390_drop_packet(wire_length); return; }
    packet = pbuf_alloc(PBUF_RAW, ethernet_state.rx_len, PBUF_POOL);
    if (packet == NULL) { ch390_drop_packet(wire_length); return; }
    for (part = packet; part != NULL; part = part->next) { ch390_read_mem(part->payload, part->len); }
    ch390_drop_packet(4);
    if (ch390_get_error() != CH390_ERROR_NONE) { pbuf_free(packet); return; }
    if (netif->input(packet, netif) != ERR_OK) { pbuf_free(packet); }
}
err_t ethernetif_init(struct netif *netif)
{
    netif->state = &ethernet_state;
    netif->name[0] = 'e'; netif->name[1] = 'n';
    netif->hwaddr_len = ETH_HWADDR_LEN;
    netif->mtu = 1500;
    netif->flags = NETIF_FLAG_BROADCAST | NETIF_FLAG_ETHARP;
    netif->output = etharp_output;
    netif->linkoutput = low_level_output;
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
    uint8_t mac[ETH_HWADDR_LEN], any = 0;
    uint16_t vendor, product;
    unsigned i;
    hardware_ready = 0;
    ch390_clear_error();
    ch390_software_reset();
    if (ch390_get_error() != CH390_ERROR_NONE) { return ERR_IF; }
    HAL_Delay(2);
    vendor = ch390_get_vendor_id(); product = ch390_get_product_id();
#if USB_DIAG_ENABLE
    usb_diag.vendor = vendor; usb_diag.product = product;
#endif
    if (ch390_get_error() != CH390_ERROR_NONE) { return ERR_IF; }
    if (vendor == 0 || vendor == 0xFFFF || product == 0 || product == 0xFFFF) {
        ch390_set_error(CH390_ERROR_ID); return ERR_IF;
    }
    ch390_default_config();
    if (ch390_get_error() != CH390_ERROR_NONE) { return ERR_IF; }
    ch390_int_pin_config(INCR_TYPE_PP, INCR_POL_H);
    ch390_get_mac(mac);
    if (ch390_get_error() != CH390_ERROR_NONE) { return ERR_IF; }
    for (i = 0; i < sizeof(mac); ++i) { any |= mac[i]; }
    if (any == 0 || (mac[0] & 1U)) { ch390_set_error(CH390_ERROR_MAC); return ERR_IF; }
    memcpy(netif->hwaddr, mac, sizeof(mac));
    hardware_ready = 1;
    return ERR_OK;
}
void init_lwip_netif(const ip4_addr_t *ip, const ip4_addr_t *mask, const ip4_addr_t *gateway)
{
    lwip_init();
    if (netif_add(&ch390_netif, ip, mask, gateway, NULL, ethernetif_init, ethernet_input) == NULL) { Error_Handler(); }
    netif_set_default(&ch390_netif);
    netif_set_link_down(&ch390_netif);
    netif_set_up(&ch390_netif);
}
u32_t sys_now(void) { return HAL_GetTick(); }
uint32_t sys_now_precise(void) { return HAL_GetTick(); }