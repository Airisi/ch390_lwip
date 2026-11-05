#include "main.h"
#include "control_transport.h"
#include "tcp_echo.h"
#include "lwip/tcp.h"
#include "lwip/timeouts.h"
#include "lwip/mem.h"
#include <string.h>
#include "netif/ethernetif.h"

static struct tcp_pcb *server_peer;
static struct tcp_pcb *client;
static uint8_t reconnect_pending;
static struct {
    struct pbuf *packet;
    uint16_t offset;
    uint8_t frame[8];
    uint8_t used;
    uint8_t closing;
} server_rx;
static void server_pump(struct tcp_pcb *pcb);
static void server_reset(void)
{
    if (server_rx.packet != NULL) { pbuf_free(server_rx.packet); }
    memset(&server_rx, 0, sizeof(server_rx));
    control_transport_tcp_reset();
}

static void reconnect(void *arg)
{
    (void)arg;
    reconnect_pending = 0;
    tcp_client_init();
}

static void retry_later(void)
{
    if (!reconnect_pending) {
        reconnect_pending = 1;
        sys_timeout(1000, reconnect, NULL);
    }
}

static void server_error(void *arg, err_t error)
{
    (void)error;
    if (server_peer == arg) { server_peer = NULL; server_reset(); }
}

static err_t close_peer(void *arg, struct tcp_pcb *pcb)
{
    err_t result;
    (void)arg;
    result = tcp_close(pcb);
    if (result == ERR_OK) {
        if (pcb == server_peer) { server_peer = NULL; server_reset(); }
        if (pcb == client) { client = NULL; retry_later(); }
    }
    return ERR_OK;
}

static err_t server_timer(void *arg, struct tcp_pcb *pcb)
{
    (void)arg;
    if (pcb == server_peer) { tcp_server_poll(); }
    return ERR_OK;
}

static void server_pump(struct tcp_pcb *pcb)
{
    uint16_t consumed = 0;
    int wrote = 0;
    for (;;) {
        if (server_rx.used == sizeof(server_rx.frame)) {
            if (server_rx.frame[7] == 0xFF) {
                if (control_transport_tcp_space() < sizeof(server_rx.frame) ||
                    tcp_write(pcb, server_rx.frame, sizeof(server_rx.frame), TCP_WRITE_FLAG_COPY) != ERR_OK) {
                    break;
                }
                (void)control_transport_tcp_rx(server_rx.frame, sizeof(server_rx.frame));
                server_rx.used = 0;
                wrote = 1;
            } else {
                uint8_t next = 1;
                while (next < sizeof(server_rx.frame) && server_rx.frame[next] != 0xFE) { ++next; }
                server_rx.used = (uint8_t)(sizeof(server_rx.frame) - next);
                memmove(server_rx.frame, server_rx.frame + next, server_rx.used);
            }
        }
        if (server_rx.packet == NULL || server_rx.offset == server_rx.packet->tot_len) { break; }
        {
            uint8_t byte = pbuf_get_at(server_rx.packet, server_rx.offset++);
            ++consumed;
            if (server_rx.used != 0 || byte == 0xFE) { server_rx.frame[server_rx.used++] = byte; }
        }
    }
    if (server_rx.packet != NULL && server_rx.offset == server_rx.packet->tot_len) {
        pbuf_free(server_rx.packet);
        server_rx.packet = NULL;
        server_rx.offset = 0;
    }
    if (consumed != 0) { tcp_recved(pcb, consumed); }
    if (wrote) { (void)tcp_output(pcb); }
}

void tcp_server_poll(void)
{
    struct tcp_pcb *pcb = server_peer;
    if (pcb == NULL) { return; }
    server_pump(pcb);
    (void)tcp_output(pcb);
    if (server_rx.closing && server_rx.packet == NULL && server_rx.used < 8 &&
        control_transport_tcp_idle()) {
        (void)close_peer(NULL, pcb);
    }
}

static err_t server_receive(void *arg, struct tcp_pcb *pcb, struct pbuf *packet, err_t error)
{
    (void)arg;
    if (packet == NULL) {
        server_rx.closing = 1;
        tcp_server_poll();
        return ERR_OK;
    }
    if (error != ERR_OK) { return error; }
    if (server_rx.packet != NULL) { return ERR_MEM; }
    server_rx.packet = packet; /* Own the pbuf after returning ERR_OK. */
    server_rx.offset = 0;
    server_pump(pcb);
    return ERR_OK;
}

static err_t accept_peer(void *arg, struct tcp_pcb *pcb, err_t error)
{
    (void)arg;
    if (error != ERR_OK) { return error; }
    if (server_peer != NULL) { tcp_abort(pcb); return ERR_ABRT; }
    server_reset();
    server_peer = pcb;
    tcp_arg(pcb, pcb);
    tcp_err(pcb, server_error);
    tcp_recv(pcb, server_receive);
    tcp_poll(pcb, server_timer, 2);
    return ERR_OK;
}

void tcp_server_init(void)
{
    struct tcp_pcb *pcb = tcp_new();
    struct tcp_pcb *listener;
    if (pcb == NULL) { Error_Handler(); }
    if (tcp_bind(pcb, IP_ADDR_ANY, 23000) != ERR_OK) { tcp_abort(pcb); Error_Handler(); }
    listener = tcp_listen(pcb);
    if (listener == NULL) { tcp_abort(pcb); Error_Handler(); }
    tcp_accept(listener, accept_peer);
}

static void client_error(void *arg, err_t error)
{
    (void)error;
    /* An older closing PCB must not clear a newly connected client. */
    if (client == arg) {
        client = NULL;
        retry_later();
    }
}

static err_t client_receive(void *arg, struct tcp_pcb *pcb, struct pbuf *packet, err_t error)
{
    err_t sent;
    void *payload;
    void *copy = NULL;
    (void)arg;
    if (packet == NULL) {
        tcp_poll(pcb, close_peer, 2);
        return close_peer(NULL, pcb);
    }
    if (error != ERR_OK) { return error; }
    payload = packet->payload;
    if (packet->len != packet->tot_len) {
        copy = mem_malloc(packet->tot_len);
        if (copy == NULL) { return ERR_MEM; }
        pbuf_copy_partial(packet, copy, packet->tot_len, 0);
        payload = copy;
    }
    sent = tcp_write(pcb, payload, packet->tot_len, TCP_WRITE_FLAG_COPY);
    if (copy != NULL) { mem_free(copy); }
    if (sent != ERR_OK) { return sent; }
    tcp_recved(pcb, packet->tot_len);
    pbuf_free(packet);
    return ERR_OK;
}

static err_t connected(void *arg, struct tcp_pcb *pcb, err_t error)
{
    (void)arg;
    if (error == ERR_OK) { tcp_recv(pcb, client_receive); }
    return error;
}

void tcp_client_init(void)
{
    ip_addr_t address;
    if (client != NULL) { return; }
    if (!netif_is_up(&ch390_netif) || !netif_is_link_up(&ch390_netif) ||
        ip4_addr_isany_val(*netif_ip4_addr(&ch390_netif))) { retry_later(); return; }
    client = tcp_new();
    if (client == NULL) { retry_later(); return; }
    IP_ADDR4(&address, 192, 168, 1, 100);
    tcp_arg(client, client);
    tcp_err(client, client_error);
    if (tcp_connect(client, &address, 2200, connected) != ERR_OK) {
        struct tcp_pcb *failed = client;
        client = NULL;
        tcp_err(failed, NULL);
        tcp_abort(failed);
        retry_later();
    }
}
