#ifndef F072_LWIPOPTS_H
#define F072_LWIPOPTS_H

/* F072 has 16 KB SRAM. Keep these pools independent of the G070 project. */
#define NO_SYS                         1
#define SYS_LIGHTWEIGHT_PROT           0
#define MEM_ALIGNMENT                  4
#define MEM_SIZE                       2560
#define MEMP_NUM_PBUF                  8
#define MEMP_NUM_RAW_PCB               0
#define MEMP_NUM_UDP_PCB               4
#define MEMP_NUM_TCP_PCB               3
#define MEMP_NUM_TCP_PCB_LISTEN        1
#define MEMP_NUM_TCP_SEG               12
#define MEMP_NUM_SYS_TIMEOUT           12
/* Two TCP peers may each retain a packet under backpressure. Leave another
   RX buffer for ACKs and DHCP instead of exhausting a two-packet pool. */
#define PBUF_POOL_SIZE                 3
#define PBUF_POOL_BUFSIZE              1520

#define LWIP_IPV4                      1
#define LWIP_IPV6                      0
#define LWIP_TCP                       1
#define LWIP_UDP                       1
#define LWIP_RAW                       0
#define LWIP_ICMP                      1
#define LWIP_ARP                       1
#define LWIP_DHCP                      1
#define DHCP_DOES_ARP_CHECK            1
#define LWIP_AUTOIP                    0
#define LWIP_DNS                       0
#define LWIP_IGMP                      0
#define LWIP_NETCONN                   0
#define LWIP_SOCKET                    0
#define LWIP_NETIF_API                 0
#define LWIP_NETIF_LINK_CALLBACK       1
#define LWIP_NETIF_STATUS_CALLBACK     1
#define LWIP_NETIF_EXT_STATUS_CALLBACK 0
#define LWIP_HAVE_LOOPIF               0
#define LWIP_NETIF_LOOPBACK            0
#define LWIP_STATS                     0
#define MIB2_STATS                     0
#define LWIP_SNMP                      0
#define LWIP_MDNS_RESPONDER            0
#define LWIP_NUM_NETIF_CLIENT_DATA     0
#define LWIP_NETIF_HOSTNAME            0
#define IP_FORWARD                    0
#define IP_REASSEMBLY                  0
#define IP_FRAG                        0
#define ARP_TABLE_SIZE                 5
#define ARP_QUEUEING                   0
#define TCP_MSS                        536
#define TCP_WND                        (2 * TCP_MSS)
#define TCP_SND_BUF                    (2 * TCP_MSS)
#define TCP_SND_QUEUELEN               8
#define TCP_SNDLOWAT                   TCP_MSS
#define TCP_QUEUE_OOSEQ                0
#define TCP_LISTEN_BACKLOG             0
#define LWIP_ALTCP                     0

#endif
