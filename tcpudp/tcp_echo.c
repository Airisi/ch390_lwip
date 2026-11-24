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
#include "lwip/opt.h"
#include "lwip/debug.h"
#include "lwip/stats.h"
#include "lwip/tcp.h"
#include "lwip/timeouts.h"

#if LWIP_TCP && LWIP_CALLBACK_API


uint8_t tcp_recv_arr[8]={0};      //数据接收缓冲区
uint8_t tcp_recv_flag=0;
    
//static int validate_data_integrity(void *data, u16_t len);
/*********************** TCP Server ***********************/
static struct tcp_pcb *tcp_server_pcb;

/**
 * @brief Send back the received data
 */
static err_t
tcp_server_recv(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err)
{
    if (p != NULL)
    {
      err_t err;
      tcp_recved(tpcb, p->tot_len);          //TCP 接收数据
      // 过滤条件1：检查数据长度，只处理长度恰好为8字节的数据
     if (p->tot_len != 8)                  
    {
        pbuf_free(p);
        return ERR_OK;
    }
     // 过滤条件2：检查数据内容（示例：第一个字节必须为0xFE）
     u8_t first_byte = *((u8_t *)p->payload);
     if (first_byte != 0xFE) 
     {     
       pbuf_free(p);
       return ERR_OK;
      }
     else
     {
       pbuf_copy_partial(p, tcp_recv_arr, sizeof(tcp_recv_arr), 0);
       tcp_recv_flag=1;  
     }

    //数据返回
   do{
        err = tcp_write(tpcb, p->payload, p->tot_len, 1);
     }while(err != ERR_OK);
        pbuf_free(p);
        return ERR_OK;
    }
    else
    {
        /* Peer FIN: release this connection through the lwIP close handshake.
           Returning ERR_BUF here leaves the PCB permanently in CLOSE_WAIT. */
        return tcp_close(tpcb);
    }

//   client_context_t *ctx = (client_context_t *)arg;
//    
//    if (p == NULL) {
//        // 连接已关闭
//        printf("Client disconnected.\n");
//        if (ctx != NULL) {
//            free(ctx);
//            tcp_arg(tpcb, NULL);
//        }
//        return ERR_OK;
//    }
//    tcp_recved(tpcb, p->tot_len);
//    // 过滤条件1：检查数据长度，只处理长度恰好为8字节的数据
//    if (p->tot_len != 8) 
//    {
////    printf("Filtered: Packet length %d != 8, dropped.\n", p->tot_len);
//        pbuf_free(p);
//        return ERR_OK;
//     }

//    // 过滤条件2：检查数据内容（示例：第一个字节必须为0xFE）
//    u8_t first_byte = *((u8_t *)p->payload);
//    if (first_byte != 0xFE) {
//   //     printf("Filtered: Packet header 0x%02X != 0xAA, dropped.\n", first_byte);
//        pbuf_free(p);
//        return ERR_OK;
//    }

//    // 过滤条件3：检查数据校验和（可选）
//    if (!validate_data_integrity(p->payload, p->tot_len))
//    {
// //       printf("Filtered: Data integrity check failed.\n");
//        pbuf_free(p);
//        return ERR_OK;
//    }

//    // 所有过滤条件通过，将数据拷贝到自定义数组arr中
//    u16_t copied_len = pbuf_copy_partial(p, arr, sizeof(arr), 0);
//    
////    printf("Data received and stored in arr: ");
////    for(int i = 0; i < copied_len; i++) {
////        printf("%02X ", arr[i]);
////    }
////    printf("\n");
//    
//    // 标记数据已接收
//    if (ctx != NULL) 
//    {
//    //    ctx->data_ready = 1;
//        ctx->data_length = copied_len;
//        tcp_recv_flag= 1;
//        
//    }

//    // 重要：通知协议栈数据已处理，更新接收窗口
////    tcp_recved(tpcb, p->tot_len);
//    pbuf_free(p);
//    
//    return ERR_OK;
}

// 数据完整性校验函数（可选）
//static int validate_data_integrity(void *data, u16_t len)
//{
//    // 这里可以实现CRC校验、校验和等验证逻辑
//    // 示例：简单的奇偶校验
//    u8_t *bytes = (u8_t *)data;
//    u8_t parity = 0;
//    for(int i = 0; i < len; i++) {
//        parity ^= bytes[i];
//    }
//    return (parity == 0); // 示例条件
//}
   


/**
 * @brief Accept connection
 */
static err_t 
 tcp_server_accept(void *arg, struct tcp_pcb *newpcb, err_t err)
{
    tcp_recv(newpcb, tcp_server_recv);
    return ERR_OK;
}

/**
 * @brief Initialize TCP server
 *        Local port: 2300
 */
void tcp_server_init(void)
{
    tcp_server_pcb = tcp_new_ip_type(IPADDR_TYPE_ANY);
    tcp_bind(tcp_server_pcb, IP_ANY_TYPE, 23000);
    tcp_server_pcb = tcp_listen(tcp_server_pcb);
    tcp_accept(tcp_server_pcb, tcp_server_accept);
}

/*********************** TCP Client ***********************/
static struct tcp_pcb *tcp_client_pcb;
static void tcp_client_reconnect(void *arg)
{
    printf("Reconnect...\n");
    tcp_client_init();
}

/**
 * @brief Send back the received data
 */
static err_t
tcp_client_recv(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err)
{
    if (p != NULL)
    {
        tcp_recved(tpcb, p->tot_len);
        tcp_write(tpcb, p->payload, p->tot_len, 1);
        pbuf_free(p);
    }
    else if (err == ERR_OK)
    {
        tcp_close(tpcb);
        printf("Disconnected\r\n");
        // Reconnect after 1 second
        sys_timeout(1000, tcp_client_reconnect, NULL);
    }
    return ERR_OK;
}

/**
 * @brief TCP error callback
 */
static void tcp_client_error(void *arg, err_t err)
{
    printf("Connection abort\n");
    // Reconnect after 1 second
    sys_timeout(1000, tcp_client_reconnect, NULL);
}

/**
 * @brief TCP connect callback
 */
static err_t tcp_client_connected(void *arg, struct tcp_pcb *pcb, err_t err)
{
    printf("Connected\r\n");
    tcp_recv(pcb, tcp_client_recv);

    return ERR_OK;
}

/**
 * @brief Initialize TCP client
 *        Server IP: 192.168.1.100, Port: 2200
 */
void tcp_client_init(void)
{
    extern struct netif ch390_netif;
    ip4_addr_t server_ip;
    uint16_t server_port = 2200;
    IP4_ADDR(&server_ip, 192, 168, 1, 100);

    tcp_client_pcb = tcp_new();
    if (tcp_client_pcb == NULL)
    {
        printf("tcp_new error\n");
        return;
    }

    tcp_bind(tcp_client_pcb, IP_ANY_TYPE, 0);
    tcp_err(tcp_client_pcb, tcp_client_error);
    tcp_connect(tcp_client_pcb, (const ip_addr_t *)&server_ip,
                server_port, tcp_client_connected);
}

#endif /* LWIP_TCP && LWIP_CALLBACK_API */
