#include "main.h"
#include "udp_echo.h"
#include "lwip/udp.h"
#include <string.h>

static void discover(void *arg, struct udp_pcb *pcb, struct pbuf *packet,
                     const ip_addr_t *address, u16_t port)
{
    uint8_t request[8];
    (void)arg;
    if (packet == NULL) { return; }
    if (packet->tot_len == sizeof(request)) {
        pbuf_copy_partial(packet, request, sizeof(request), 0);
        if (memcmp(request, "DISCOVER", sizeof(request)) == 0) {
            static const char response[] = "DISCOVERHOST";
            struct pbuf *reply = pbuf_alloc(PBUF_TRANSPORT, sizeof(response) - 1U, PBUF_RAM);
            if (reply != NULL) {
                pbuf_take(reply, response, sizeof(response) - 1U);
                (void)udp_sendto(pcb, reply, address, port);
                pbuf_free(reply);
            }
        }
    }
    pbuf_free(packet);
}

void udp_discover_init(void)
{
    struct udp_pcb *pcb = udp_new();
    if (pcb == NULL) { Error_Handler(); }
    if (udp_bind(pcb, IP_ADDR_ANY, 8888) != ERR_OK) { udp_remove(pcb); Error_Handler(); }
    udp_recv(pcb, discover, NULL);
}
