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

#include "lwip/opt.h"

#include "lwip/def.h"
#include "lwip/init.h"
#include "lwip/mem.h"
#include "lwip/pbuf.h"
#include "lwip/stats.h"
#include "lwip/snmp.h"
#include "lwip/ethip6.h"
#include "lwip/etharp.h"
#include "netif/ppp/pppoe.h"
#include "netif/ethernet.h"
#include "netif/ethernetif.h"

#include "CH390_Interface.h"
#include "stm32f0xx_hal.h"


/* Define those to better describe your network interface. */
#define IFNAME0 'e'
#define IFNAME1 'n'


struct netif ch390_netif;

/* Forward declarations. */
void ethernetif_input(struct netif *netif);

/**
 * In this function, the hardware should be initialized.
 * Called from ethernetif_init().
 *
 * @param netif the already initialized lwip network interface structure
 *        for this ethernetif
 */
static void low_level_init(struct netif *netif)
{
    //   struct ethernetif *ethernetif = netif->state;
    ch390_default_config();
    /* PD8 is wired as a high-active interrupt input on this board. */
    ch390_int_pin_config(INCR_TYPE_PP, INCR_POL_H);

    /* set MAC hardware address length */
    netif->hwaddr_len = ETHARP_HWADDR_LEN;

    /* set MAC hardware address */
    ch390_get_mac(netif->hwaddr);

    /* maximum transfer unit */
    netif->mtu = 1500;

    /* device capabilities */
    /* don't set NETIF_FLAG_ETHARP if this device is not an ethernet one */
    netif->flags = NETIF_FLAG_BROADCAST | NETIF_FLAG_ETHARP;

#if LWIP_IPV6 && LWIP_IPV6_MLD
    /*
     * For hardware/netifs that implement MAC filtering.
     * All-nodes link-local is handled by default, so we must let the hardware know
     * to allow multicast packets in.
     * Should set mld_mac_filter previously. */
    if (netif->mld_mac_filter != NULL)
    {
        ip6_addr_t ip6_allnodes_ll;
        ip6_addr_set_allnodes_linklocal(&ip6_allnodes_ll);
        netif->mld_mac_filter(netif, &ip6_allnodes_ll, NETIF_ADD_MAC_FILTER);
    }
#endif /* LWIP_IPV6 && LWIP_IPV6_MLD */

    /* Do whatever else is needed to initialize interface. */
}

/**
 * This function should do the actual transmission of the packet. The packet is
 * contained in the pbuf that is passed to the function. This pbuf
 * might be chained.
 *
 * @param netif the lwip network interface structure for this ethernetif
 * @param p the MAC packet to send (e.g. IP packet including MAC addresses and type)
 * @return ERR_OK if the packet could be sent
 *         an err_t value if the packet couldn't be sent
 *
 * @note Returning ERR_MEM here if a DMA queue of your MAC is full can lead to
 *       strange results. You might consider waiting for space in the DMA queue
 *       to become available since the stack doesn't retry to send a packet
 *       dropped because of memory failure (except for the TCP timers).
 */

static err_t low_level_output(struct netif *netif, struct pbuf *p)
{
    // struct ethernetif *ethernetif = netif->state;
    struct pbuf *q;

#if ETH_PAD_SIZE
    pbuf_remove_header(p, ETH_PAD_SIZE); /* drop the padding word */
#endif

    for (q = p; q != NULL; q = q->next)
    {
        /* Send the data from the pbuf to the interface, one pbuf at a
           time. The size of the data in each pbuf is kept in the ->len
           variable. */
        ch390_write_mem(q->payload, q->len);
    }

    // Wait until last transmit complete
    while(ch390_read_reg(CH390_TCR) & TCR_TXREQ);
    // Set current packet length
    ch390_write_reg(CH390_TXPLL, p->tot_len & 0xff);
    ch390_write_reg(CH390_TXPLH, (p->tot_len >> 8) & 0xff);
    // Issue transmit request
    ch390_send_request();

    LINK_STATS_INC(link.xmit);

    return ERR_OK;
}

/**
 * Should allocate a pbuf and transfer the bytes of the incoming
 * packet from the interface into the pbuf.
 *
 * @param netif the lwip network interface structure for this ethernetif
 * @return a pbuf filled with the received packet (including MAC header)
 *         NULL on memory error
 */
static struct pbuf *low_level_input(struct netif *netif)
{
    struct ethernetif *ethernetif = netif->state;
    struct pbuf *p, *q;
    u16_t len;

    u8_t rx_ready;
    u8_t ReceiveData[4];

    ch390_read_reg(CH390_MRCMDX);
    rx_ready = ch390_read_reg(CH390_MRCMDX);

    if (rx_ready & CH390_PKT_ERR)
    {
        // Reset RX FIFO pointer
        ch390_write_reg(CH390_RCR, 0);        //RX disable
        ch390_write_reg(CH390_MPTRCR, 0x01);  //Reset RX FIFO pointer
        ch390_write_reg(CH390_MRRH, 0x0c);
        HAL_Delay(1);
        ch390_write_reg(CH390_RCR, RCR_RXEN | RCR_DIS_CRC); //RX Enable
        LINK_STATS_INC(link.drop);
        ethernetif->rx_len = 0;
        return NULL;
    }
    if (!(rx_ready & CH390_PKT_RDY))
    {
        ethernetif->rx_len = 0;
        return NULL;
    }
    ch390_read_mem(ReceiveData, 4);
    ethernetif->rx_status = ReceiveData[1];
    /* Ignore the last 4 bytes CRC */
    ethernetif->rx_len = (ReceiveData[2] | (ReceiveData[3] << 8)) - 4;
    /* Obtain the size of the packet and put it into the "len"
       variable. */
    len = ethernetif->rx_len;
#if ETH_PAD_SIZE
    len += ETH_PAD_SIZE; /* allow room for Ethernet padding */
#endif

    /* We allocate a pbuf chain of pbufs from the pool. */
    p = pbuf_alloc(PBUF_RAW, len, PBUF_POOL);

    if (p != NULL)
    {
#if ETH_PAD_SIZE
        pbuf_remove_header(p, ETH_PAD_SIZE); /* drop the padding word */
#endif

        /* We iterate over the pbuf chain until we have read the entire
         * packet into the pbuf. */
        for (q = p; q != NULL; q = q->next)
        {
            /* Read enough bytes to fill this pbuf in the chain. The
             * available data in the pbuf is given by the q->len
             * variable.
             * This does not necessarily have to be a memcpy, you can also preallocate
             * pbufs for a DMA-enabled MAC and after receiving truncate it to the
             * actually received size. In this case, ensure the tot_len member of the
             * pbuf is the sum of the chained pbuf len members.
             */
            ch390_read_mem(q->payload, q->len);
           
        }

#if ETH_PAD_SIZE
        pbuf_add_header(p, ETH_PAD_SIZE); /* reclaim the padding word */
#endif
        /* Skip last 4 bytes CRC */
        ch390_drop_packet(4);
        LINK_STATS_INC(link.recv);
    }
    else
    {
        ch390_drop_packet(len + 4);
        LINK_STATS_INC(link.memerr);
        LINK_STATS_INC(link.drop);
        MIB2_STATS_NETIF_INC(netif, ifindiscards);
    }

    return p;
}

/**
 * This function should be called when a packet is ready to be read
 * from the interface. It uses the function low_level_input() that
 * should handle the actual reception of bytes from the network
 * interface. Then the type of the received packet is determined and
 * the appropriate input function is called.
 *
 * @param netif the lwip network interface structure for this ethernetif
 */
void ethernetif_input(struct netif *netif)
{
    // struct ethernetif *ethernetif;
    // struct eth_hdr *ethhdr;
    struct pbuf *p;

    // ethernetif = netif->state;

    /* move received packet into a new pbuf */
    p = low_level_input(netif);
    /* if no packet could be read, silently ignore this */
    if (p != NULL)
    {
        /* pass all packets to ethernet_input, which decides what packets it supports */
        if (netif->input(p, netif) != ERR_OK)
        {
            LWIP_DEBUGF(NETIF_DEBUG, ("ethernetif_input: IP input error\n"));
            pbuf_free(p);
            p = NULL;
        }
    }
}

/**
 * Should be called at the beginning of the program to set up the
 * network interface. It calls the function low_level_init() to do the
 * actual setup of the hardware.
 *
 * This function should be passed as a parameter to netif_add().
 *
 * @param netif the lwip network interface structure for this ethernetif
 * @return ERR_OK if the loopif is initialized
 *         ERR_MEM if private data couldn't be allocated
 *         any other err_t on error
 */
err_t ethernetif_init(struct netif *netif)
{
    struct ethernetif *ethernetif;

    LWIP_ASSERT("netif != NULL", (netif != NULL));

    ethernetif = mem_malloc(sizeof(struct ethernetif));
    if (ethernetif == NULL)
    {
        LWIP_DEBUGF(NETIF_DEBUG, ("ethernetif_init: out of memory\n"));
        return ERR_MEM;
    }

#if LWIP_NETIF_HOSTNAME
    /* Initialize interface hostname */
    netif->hostname = "lwip";
#endif /* LWIP_NETIF_HOSTNAME */

    /*
     * Initialize the snmp variables and counters inside the struct netif.
     * The last argument should be replaced with your link speed, in units
     * of bits per second.
     */
    MIB2_INIT_NETIF(netif, snmp_ifType_ethernet_csmacd, 100);

    netif->state = ethernetif;
    netif->name[0] = IFNAME0;
    netif->name[1] = IFNAME1;
    /* We directly use etharp_output() here to save a function call.
     * You can instead declare your own function an call etharp_output()
     * from it if you have to do some checks before sending (e.g. if link
     * is available...) */
#if LWIP_IPV4
    netif->output = etharp_output;
#endif /* LWIP_IPV4 */
#if LWIP_IPV6
    netif->output_ip6 = ethip6_output;
#endif /* LWIP_IPV6 */
    netif->linkoutput = low_level_output;

    // ethernetif->ethaddr = (struct eth_addr *)&(netif->hwaddr[0]);

    /* initialize the hardware */
    low_level_init(netif);

    return ERR_OK;
}

/**
 * Initialize LwIP with ch390_netif
 */
void init_lwip_netif(const ip4_addr_t *ipaddr, const ip4_addr_t *netmask, const ip4_addr_t *gw)
{
    /* Initialize the LwIP stack without RTOS */
    lwip_init();

    /* add the network interface (IPv4/IPv6) without RTOS */
   netif_add(&ch390_netif, ipaddr, netmask, gw, NULL,
              &ethernetif_init, &ethernet_input);

    /* Registers the default network interface */
    netif_set_default(&ch390_netif);
    netif_set_link_up(&ch390_netif);
    netif_set_up(&ch390_netif);
}

// 全局变量
static uint32_t lwip_tick_base = 0;
static uint16_t last_ssr = 0xFFFF;
static const uint32_t SYNC_PREDIV = 319;  // 与初始化一致

// 获取RTC亚秒计数器
static inline uint16_t rtc_get_ssr(void)
{
    return (uint16_t)(RTC->SSR);
}

// 获取当前毫秒数（用于sys_now）
u32_t sys_now(void)
{
    uint16_t current_ssr = rtc_get_ssr();
    uint16_t ssr_diff;
    uint32_t current_ms;
    
    // SSR寄存器是递减计数器，范围0-255
    // 32768Hz / (127+1) / (255+1) = 1Hz (1秒)
    // SSR每255-0递减一次，约7.8ms
    
    // 计算差值（处理翻转）
    if (current_ssr <= last_ssr)
    {
        ssr_diff = last_ssr - current_ssr;
    }
    else
    {
        // SSR从0翻转到255
        ssr_diff = (SYNC_PREDIV + 1) + last_ssr - current_ssr;
    }
    
    // 转换为毫秒
    // 每个SSR计数 = 1000ms / 256 ≈ 3.90625ms
    // 使用整数运算提高效率
    current_ms = (ssr_diff * 1000) / (SYNC_PREDIV + 1);
    
    // 更新累计tick
    lwip_tick_base += current_ms;
    last_ssr = current_ssr;
    
    return lwip_tick_base;
}
// 更精确的版本，结合秒和亚秒
uint32_t sys_now_precise(void)
{
    static uint32_t last_seconds = 0;
    static uint32_t base_seconds = 0;
    RTC_TimeTypeDef sTime = {0};
    volatile  uint32_t  current_seconds, current_ms;
    uint16_t current_ssr;
    uint32_t ssr_ms;
    
    // 1. 直接读取RTC寄存器（避免函数调用开销）
    uint32_t tr = RTC->TR;
    uint32_t dr = RTC->DR;
    
    // 2. 解析时间
    sTime.Hours = (uint8_t)((tr & (RTC_TR_HT | RTC_TR_HU)) >> 16);
    sTime.Minutes = (uint8_t)((tr & (RTC_TR_MNT | RTC_TR_MNU)) >> 8);
    sTime.Seconds = (uint8_t)(tr & (RTC_TR_ST | RTC_TR_SU));
    
    // 3. 获取亚秒
    current_ssr = (uint16_t)(RTC->SSR);
    
    // 4. 计算当前秒数
    current_seconds = sTime.Hours * 3600 + sTime.Minutes * 60 + sTime.Seconds;
    
    // 5. 处理日期切换
    if (current_seconds < last_seconds)
    {
        base_seconds += 86400;  // 一天86400秒
    }
    last_seconds = current_seconds;
    
    // 6. 计算亚秒对应的毫秒
    // SSR: 255->0, 每个计数=1000/256≈3.90625ms
    ssr_ms = ((SYNC_PREDIV - current_ssr) * 1000) / (SYNC_PREDIV + 1);
    
    // 7. 总毫秒数
    return ((base_seconds + current_seconds) * 1000) + ssr_ms;
}



