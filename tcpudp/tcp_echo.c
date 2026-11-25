/********************************** (C) COPYRIGHT *****************************
 * File Name          : tcp_echo.c
 * Author             : WCH
 * Version            : V1.0
 * Date               : 2024/08/20
 * Description        : TCP echo example.
 ******************************************************************************
 * Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
 * Attention: This software (modified or not) and binary are used for
 * microcontroller manufactured by Nanjing Qinheng Microelectronics.
 ******************************************************************************/

#include "tcp_echo.h"
#include "control_transport.h"
#include "netif/ethernetif.h"
#include "lwip/tcp.h"
#include "lwip/mem.h"
#include "lwip/sys.h"
#include <string.h>

/* Each connection can own its entire advertised window without retaining
   PBUF_POOL buffers. ACKs must remain receivable under TX backpressure. */
#define RX_CAPACITY TCP_WND
#define RETRY_MS 1000U
#define WORK_BUDGET 4U
#if TCP_WND > 65535 || LWIP_WND_SCALE
#error This bounded receive queue requires an unscaled TCP window
#endif

typedef struct Peer {
    struct Peer *next;
    struct tcp_pcb *pcb;
    uint16_t head, tail, count;
    uint8_t frame[8], used, closing, outgoing;
    uint8_t rx[RX_CAPACITY];
} Peer;

uint8_t tcp_recv_arr[8], tcp_recv_flag;
static Peer *peers, *client;
static struct tcp_pcb *listener;
static uint32_t client_retry_at, listener_retry_at;
static uint8_t client_retry, listener_retry;

static int due(uint32_t now, uint32_t deadline)
{
    return (int32_t)(now - deadline) >= 0;
}
static void retry_client(void)
{
    client_retry_at = sys_now() + RETRY_MS;
    client_retry = 1;
}
static void release_peer(Peer *peer)
{
    Peer **link = &peers;
    while (*link != NULL && *link != peer) { link = &(*link)->next; }
    if (*link == peer) { *link = peer->next; }
    if (client == peer) { client = NULL; retry_client(); }
    mem_free(peer);
}
static void peer_error(void *arg, err_t error)
{
    (void)error;
    release_peer((Peer *)arg); /* lwIP has already freed the PCB. */
}
static void abort_peer(Peer *peer)
{
    /* The error callback removes the peer and schedules outgoing retries. */
    tcp_abort(peer->pcb);
}
static void close_peer(Peer *peer)
{
    struct tcp_pcb *pcb = peer->pcb;
    err_t result;
    /* A successfully closing PCB may outlive its application context. */
    tcp_arg(pcb, NULL);
    tcp_err(pcb, NULL);
    tcp_recv(pcb, NULL);
    result = tcp_close(pcb);
    if (result == ERR_OK) { release_peer(peer); }
    else {
        tcp_arg(pcb, peer);
        tcp_err(pcb, peer_error);
        /* RX is already finished; foreground retries the close. */
    }
}
static err_t receive_peer(void *arg, struct tcp_pcb *pcb, struct pbuf *packet, err_t error)
{
    Peer *peer = (Peer *)arg;
    uint16_t offset = 0;
    (void)pcb;
    if (packet == NULL) { peer->closing = 1; return ERR_OK; }
    if (error != ERR_OK || packet->tot_len > RX_CAPACITY - peer->count - peer->used) {
        /* This cannot occur for an in-window packet. Never retain pool pbufs. */
        pbuf_free(packet);
        abort_peer(peer);
        return ERR_ABRT;
    }
    while (offset < packet->tot_len) {
        uint16_t amount = (uint16_t)(RX_CAPACITY - peer->head);
        uint16_t remaining = (uint16_t)(packet->tot_len - offset);
        if (amount > remaining) { amount = remaining; }
        pbuf_copy_partial(packet, peer->rx + peer->head, amount, offset);
        peer->head = (uint16_t)((peer->head + amount) % RX_CAPACITY);
        peer->count += amount;
        offset += amount;
    }
    pbuf_free(packet);
    /* Window credit is returned only after foreground consumption. */
    return ERR_OK;
}
static Peer *attach_peer(struct tcp_pcb *pcb, uint8_t outgoing)
{
    Peer *peer = (Peer *)mem_malloc(sizeof(Peer));
    if (peer == NULL) { return NULL; }
    memset(peer, 0, sizeof(*peer));
    peer->pcb = pcb;
    peer->outgoing = outgoing;
    peer->next = peers;
    peers = peer;
    tcp_arg(pcb, peer);
    tcp_err(pcb, peer_error);
    tcp_recv(pcb, receive_peer);
    return peer;
}
static err_t accept_peer(void *arg, struct tcp_pcb *pcb, err_t error)
{
    (void)arg;
    if (error != ERR_OK || attach_peer(pcb, 0) == NULL) {
        tcp_abort(pcb);
        return ERR_ABRT;
    }
    return ERR_OK;
}
static uint8_t take_byte(Peer *peer)
{
    uint8_t byte = peer->rx[peer->tail];
    peer->tail = (uint16_t)((peer->tail + 1U) % RX_CAPACITY);
    --peer->count;
    return byte;
}
static void pump_peer(Peer *peer)
{
    unsigned work = 0;
    struct tcp_pcb *pcb = peer->pcb;
    if (pcb->state != ESTABLISHED && pcb->state != CLOSE_WAIT) { return; }
    if (peer->outgoing) {
        while (peer->count != 0 && work++ < WORK_BUDGET) {
            uint16_t amount = (uint16_t)(RX_CAPACITY - peer->tail);
            if (amount > peer->count) { amount = peer->count; }
            if (amount > 64U) { amount = 64U; }
            if (tcp_write(pcb, peer->rx + peer->tail, amount, TCP_WRITE_FLAG_COPY) != ERR_OK) { break; }
            peer->tail = (uint16_t)((peer->tail + amount) % RX_CAPACITY);
            peer->count -= amount;
            tcp_recved(pcb, amount);
        }
    } else {
        while (work < WORK_BUDGET) {
            if (peer->used == 8U) {
                if (peer->frame[7] == 0xFF) {
                    if (tcp_write(pcb, peer->frame, 8, TCP_WRITE_FLAG_COPY) != ERR_OK) { break; }
                    memcpy(tcp_recv_arr, peer->frame, 8);
                    tcp_recv_flag = 1;
                    tcp_recv_data_process();
                    tcp_recv_flag = 0;
                    tcp_recved(pcb, 8);
                    peer->used = 0;
                    ++work;
                } else {
                    uint8_t next = 1;
                    while (next < 8U && peer->frame[next] != 0xFE) { ++next; }
                    peer->used = (uint8_t)(8U - next);
                    memmove(peer->frame, peer->frame + next, peer->used);
                    tcp_recved(pcb, next);
                }
            }
            if (peer->count == 0) { break; }
            {
                uint8_t byte = take_byte(peer);
                if (peer->used != 0 || byte == 0xFE) { peer->frame[peer->used++] = byte; }
                else { tcp_recved(pcb, 1); }
            }
        }
    }
    (void)tcp_output(pcb);
    if (peer->closing && peer->count == 0 && peer->used < 8U) {
        /* Discard an incomplete final frame, while preserving complete ones. */
        if (peer->used != 0) { tcp_recved(pcb, peer->used); peer->used = 0; }
        close_peer(peer);
    }
}

void tcp_server_init(void)
{
    struct tcp_pcb *pcb, *listening;
    if (listener != NULL) { return; }
    listener_retry = 0;
    pcb = tcp_new();
    if (pcb != NULL) {
        if (tcp_bind(pcb, IP_ADDR_ANY, 23000) == ERR_OK) {
            listening = tcp_listen(pcb);
            if (listening != NULL) {
                listener = listening;
                tcp_accept(listener, accept_peer);
                return;
            }
        }
        tcp_abort(pcb);
    }
    listener_retry_at = sys_now() + RETRY_MS;
    listener_retry = 1;
}
static err_t connected(void *arg, struct tcp_pcb *pcb, err_t error)
{
    (void)pcb;
    if (error != ERR_OK) { abort_peer((Peer *)arg); return ERR_ABRT; }
    return ERR_OK;
}
void tcp_client_init(void)
{
    struct tcp_pcb *pcb;
    ip_addr_t address;
    Peer *peer;
    client_retry = 0;
    if (client != NULL) { return; }
    if (!netif_is_up(&ch390_netif) || !netif_is_link_up(&ch390_netif) ||
        ip4_addr_isany_val(*netif_ip4_addr(&ch390_netif))) { retry_client(); return; }
    pcb = tcp_new();
    if (pcb == NULL) { retry_client(); return; }
    if (tcp_bind(pcb, IP_ADDR_ANY, 0) != ERR_OK) { tcp_abort(pcb); retry_client(); return; }
    peer = attach_peer(pcb, 1);
    if (peer == NULL) { tcp_abort(pcb); retry_client(); return; }
    client = peer;
    IP_ADDR4(&address, 192, 168, 1, 100);
    if (tcp_connect(pcb, &address, 2200, connected) != ERR_OK) { abort_peer(peer); }
}
void tcp_app_poll(void)
{
    Peer *peer = peers;
    uint32_t now;
    while (peer != NULL) {
        Peer *next = peer->next;
        pump_peer(peer);
        peer = next;
    }
    now = sys_now();
    if (listener_retry && due(now, listener_retry_at)) { tcp_server_init(); }
    if (client_retry && due(now, client_retry_at)) { tcp_client_init(); }
}

