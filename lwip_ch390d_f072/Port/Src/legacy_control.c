/* Legacy IO dispatchers intentionally copied verbatim from the G070 main.c.
 * The old mappings are NOT adapted to F072 SPI/USB pins in this migration. */
#include "main.h"
#include "control_transport.h"
#include "flash_net_cfg.h"
#include "netif/ethernetif.h"
#include "lwip/dhcp.h"
#include <string.h>

volatile uint8_t net_mode;
static uint8_t static_ip_frame;
static uint8_t static_ip[4];
static uint8_t static_mask[4];
static uint8_t static_gw[4];


/*tcp接收数据处理*/  
void tcp_recv_data_process(void)   
{
  if((tcp_recv_arr[0]==0xFE)&&(tcp_recv_arr[7]==0xFF))
   {
     switch(tcp_recv_arr[1])
     {
        case 1:                  
       if(tcp_recv_arr[2]==1)   
       {                                  /*打开继电器1*/
          HAL_GPIO_WritePin(GPIOC,GPIO_PIN_13,GPIO_PIN_SET);
       }
       else                                /*关闭继电器1*/ 
       {
         HAL_GPIO_WritePin(GPIOC,GPIO_PIN_13,GPIO_PIN_RESET);
       
       }
          break;
       case 2:                          
         if(tcp_recv_arr[2]==1)   
       {                                     /*打开继电器2*/
          HAL_GPIO_WritePin(GPIOC,GPIO_PIN_12,GPIO_PIN_SET);
       }
       else                                   /*关闭继电器2*/ 
       {
         HAL_GPIO_WritePin(GPIOC,GPIO_PIN_12,GPIO_PIN_RESET);
       
       }       
        break;
        case 3:                            
        if(tcp_recv_arr[2]==1)        
       {                                     /*打开继电器3*/ 
          HAL_GPIO_WritePin(GPIOC,GPIO_PIN_11,GPIO_PIN_SET);
       }
       else                                    /*关闭继电器3*/ 
       {
         HAL_GPIO_WritePin(GPIOC,GPIO_PIN_11,GPIO_PIN_RESET);
       
       }            
        break;
        case 4:                              
        if(tcp_recv_arr[2]==1)        
       {                                       /*打开继电器4*/ 
          HAL_GPIO_WritePin(GPIOC,GPIO_PIN_10,GPIO_PIN_SET);
       }
       else                                /*关闭继电器4*/ 
       {
         HAL_GPIO_WritePin(GPIOC,GPIO_PIN_10,GPIO_PIN_RESET);
       
       }         
        break;
       case 5:    
        if(tcp_recv_arr[2]==1)               /*打开继电器5*/ 
       {
          HAL_GPIO_WritePin(GPIOB,GPIO_PIN_9,GPIO_PIN_SET);
       }
       else                                   /*关闭继电器5*/
       {
         HAL_GPIO_WritePin(GPIOB,GPIO_PIN_9,GPIO_PIN_RESET);
       
       }              
        break; 
        case 6:      
        if(tcp_recv_arr[2]==1)               /*打开继电器6*/ 
       {
          HAL_GPIO_WritePin(GPIOB,GPIO_PIN_8,GPIO_PIN_SET);
       }
       else                                   /*关闭继电器6*/
       {
         HAL_GPIO_WritePin(GPIOB,GPIO_PIN_8,GPIO_PIN_RESET);
       
       }             
        break;
        case 7:    
       if(tcp_recv_arr[2]==1)               /*打开继电器7*/ 
       {
          HAL_GPIO_WritePin(GPIOB,GPIO_PIN_5,GPIO_PIN_SET);
       }
       else                                   /*关闭继电器7*/
       {
         HAL_GPIO_WritePin(GPIOB,GPIO_PIN_5,GPIO_PIN_RESET);
       
       }     
        break;
        case 8:     
        if(tcp_recv_arr[2]==1)               /*打开继电器8*/ 
       {
          HAL_GPIO_WritePin(GPIOB,GPIO_PIN_4,GPIO_PIN_SET);
       }
       else                                   /*关闭继电器8*/
       {
         HAL_GPIO_WritePin(GPIOB,GPIO_PIN_4,GPIO_PIN_RESET);
       
       }     
        break;
        case 9:      
         if(tcp_recv_arr[2]==1)               /*打开继电器9*/ 
       {
          HAL_GPIO_WritePin(GPIOB,GPIO_PIN_3,GPIO_PIN_SET);
       }
       else                                    /*关闭继电器9*/
       {
         HAL_GPIO_WritePin(GPIOB,GPIO_PIN_3,GPIO_PIN_RESET);
       
       }                  
        break;
        case 10:    
       if(tcp_recv_arr[2]==1)               /*打开继电器10*/ 
       {
          HAL_GPIO_WritePin(GPIOD,GPIO_PIN_4,GPIO_PIN_SET);
       }
       else                                    /*关闭继电器10*/
       {
         HAL_GPIO_WritePin(GPIOD,GPIO_PIN_4,GPIO_PIN_RESET);
       
       }       
        break; 
        case 11:    
      if(tcp_recv_arr[2]==1)                /*打开继电器11*/ 
       {
          HAL_GPIO_WritePin(GPIOD,GPIO_PIN_3,GPIO_PIN_SET);
       }
       else                                    /*关闭继电器11*/
       {
         HAL_GPIO_WritePin(GPIOD,GPIO_PIN_3,GPIO_PIN_RESET);
       
       }       
        break;
        case 12:     
       if(tcp_recv_arr[2]==1)                /*打开继电器12*/ 
       {
          HAL_GPIO_WritePin(GPIOD,GPIO_PIN_2,GPIO_PIN_SET);
       }
       else                                    /*关闭继电器12*/
       {
         HAL_GPIO_WritePin(GPIOD,GPIO_PIN_2,GPIO_PIN_RESET);
       
       }           
        break;
        case 13: 
         if(tcp_recv_arr[2]==1)                /*打开继电器13*/ 
       {
          HAL_GPIO_WritePin(GPIOD,GPIO_PIN_1,GPIO_PIN_SET);
       }
       else                                    /*关闭继电器13*/
       {
         HAL_GPIO_WritePin(GPIOD,GPIO_PIN_1,GPIO_PIN_RESET);
       
       }             
        break;
        case 14: 
        if(tcp_recv_arr[2]==1)                /*打开继电器14*/ 
       {
          HAL_GPIO_WritePin(GPIOD,GPIO_PIN_0,GPIO_PIN_SET);
       }
       else                                    /*关闭继电器14*/
       {
         HAL_GPIO_WritePin(GPIOD,GPIO_PIN_0,GPIO_PIN_RESET);
       
       }            
        break;
        case 15:  
       if(tcp_recv_arr[2]==1)                /*打开继电器15*/ 
       {
          HAL_GPIO_WritePin(GPIOC,GPIO_PIN_9,GPIO_PIN_SET);
       }
       else                                    /*关闭继电器15*/
       {
         HAL_GPIO_WritePin(GPIOC,GPIO_PIN_9,GPIO_PIN_RESET);
       
       }           
        break; 
        case 16:  
       if(tcp_recv_arr[2]==1)                /*打开继电器16*/ 
       {
          HAL_GPIO_WritePin(GPIOC,GPIO_PIN_8,GPIO_PIN_SET);
       }
       else                                    /*关闭继电器16*/
       {
         HAL_GPIO_WritePin(GPIOC,GPIO_PIN_8,GPIO_PIN_RESET);
       
       }        
      break;
     case 17:     
       if(tcp_recv_arr[2]==1)                /*打开继电器17*/ 
       {
          HAL_GPIO_WritePin(GPIOA,GPIO_PIN_15,GPIO_PIN_SET);
       }
       else                                    /*关闭继电器17*/
       {
         HAL_GPIO_WritePin(GPIOA,GPIO_PIN_15,GPIO_PIN_RESET);
       
       }          
     break;
     case 18:
      if(tcp_recv_arr[2]==1)                  /*打开继电器18*/ 
       {
          HAL_GPIO_WritePin(GPIOA,GPIO_PIN_12,GPIO_PIN_SET);
       }
       else                                    /*关闭继电器18*/
       {
         HAL_GPIO_WritePin(GPIOA,GPIO_PIN_12,GPIO_PIN_RESET);
       
       }                
     break;
     case 19: 
      if(tcp_recv_arr[2]==1)                  /*打开继电器19*/ 
       {
          HAL_GPIO_WritePin(GPIOA,GPIO_PIN_11,GPIO_PIN_SET);
       }
       else                                    /*关闭继电器19*/
       {
         HAL_GPIO_WritePin(GPIOA,GPIO_PIN_11,GPIO_PIN_RESET);
       
       }       
     break;
     case 20: 
     if(tcp_recv_arr[2]==1)                  /*打开继电器20*/ 
       {
          HAL_GPIO_WritePin(GPIOA,GPIO_PIN_10,GPIO_PIN_SET);
       }
       else                                    /*关闭继电器20*/
       {
         HAL_GPIO_WritePin(GPIOA,GPIO_PIN_10,GPIO_PIN_RESET);
       
       }        
     break; 
     case 21:    
     if(tcp_recv_arr[2]==1)                  /*打开继电器21*/ 
       {
          HAL_GPIO_WritePin(GPIOD,GPIO_PIN_9,GPIO_PIN_SET);
       }
       else                                    /*关闭继电器21*/
       {
         HAL_GPIO_WritePin(GPIOD,GPIO_PIN_9,GPIO_PIN_RESET);
       
       }       
     break;
     case 22:    
      if(tcp_recv_arr[2]==1)                  /*打开继电器22*/ 
       {
          HAL_GPIO_WritePin(GPIOD,GPIO_PIN_8,GPIO_PIN_SET);
       }
       else                                    /*关闭继电器22*/
       {
         HAL_GPIO_WritePin(GPIOD,GPIO_PIN_8,GPIO_PIN_RESET);
       
       }         
     break;
     case 23:  
      if(tcp_recv_arr[2]==1)                  /*打开继电器23*/ 
       {
          HAL_GPIO_WritePin(GPIOC,GPIO_PIN_7,GPIO_PIN_SET);
       }
       else                                    /*关闭继电器23*/
       {
         HAL_GPIO_WritePin(GPIOC,GPIO_PIN_7,GPIO_PIN_RESET);
       
       }       
     break;
     case 24:     
       if(tcp_recv_arr[2]==1)                  /*打开继电器24*/ 
      {
          HAL_GPIO_WritePin(GPIOC,GPIO_PIN_6,GPIO_PIN_SET);
      }
      else                                    /*关闭继电器24*/
      {
         HAL_GPIO_WritePin(GPIOC,GPIO_PIN_6,GPIO_PIN_RESET);
       
      }           
     break;
     case 25:  
      if(tcp_recv_arr[2]==1)                  /*打开继电器25*/ 
      {
          HAL_GPIO_WritePin(GPIOA,GPIO_PIN_9,GPIO_PIN_SET);
      }
      else                                    /*关闭继电器25*/
      {
         HAL_GPIO_WritePin(GPIOA,GPIO_PIN_9,GPIO_PIN_RESET);
       
      }            
     break; 
     case 26: 
      if(tcp_recv_arr[2]==1)                  /*打开继电器26*/ 
      {
          HAL_GPIO_WritePin(GPIOA,GPIO_PIN_8,GPIO_PIN_SET);
      }
      else                                    /*关闭继电器26*/
      {
         HAL_GPIO_WritePin(GPIOA,GPIO_PIN_8,GPIO_PIN_RESET);
       
      }           
      break;
     case 27:   
       if(tcp_recv_arr[2]==1)                  /*打开继电器27*/ 
      {
          HAL_GPIO_WritePin(GPIOB,GPIO_PIN_15,GPIO_PIN_SET);
      }
      else                                    /*关闭继电器27*/
      {
         HAL_GPIO_WritePin(GPIOB,GPIO_PIN_15,GPIO_PIN_RESET);
       
      }         
      break;
     case 28: 
       if(tcp_recv_arr[2]==1)                  /*打开继电器28*/ 
      {
          HAL_GPIO_WritePin(GPIOB,GPIO_PIN_14,GPIO_PIN_SET);
      }
      else                                    /*关闭继电器28*/
      {
         HAL_GPIO_WritePin(GPIOB,GPIO_PIN_14,GPIO_PIN_RESET);
       
      }    
     break;
     case 29:   
        if(tcp_recv_arr[2]==1)                  /*打开继电器29*/ 
      {
          HAL_GPIO_WritePin(GPIOB,GPIO_PIN_13,GPIO_PIN_SET);
      }
      else                                     /*关闭继电器29*/
      {
         HAL_GPIO_WritePin(GPIOB,GPIO_PIN_13,GPIO_PIN_RESET);
       
      }    
     break;
     case 30:     
        if(tcp_recv_arr[2]==1)                  /*打开继电器30*/ 
      {
          HAL_GPIO_WritePin(GPIOB,GPIO_PIN_12,GPIO_PIN_SET);
      }
      else                                     /*关闭继电器30*/
      {
         HAL_GPIO_WritePin(GPIOB,GPIO_PIN_12,GPIO_PIN_RESET);
       
      }      
     break;
     case 31:     
         if(tcp_recv_arr[2]==1)                  /*打开继电器31*/ 
      {
          HAL_GPIO_WritePin(GPIOB,GPIO_PIN_11,GPIO_PIN_SET);
      }
      else                                     /*关闭继电器31*/
      {
         HAL_GPIO_WritePin(GPIOB,GPIO_PIN_11,GPIO_PIN_RESET);
       
      }        
     break;
     case 32:     
         if(tcp_recv_arr[2]==1)                  /*打开继电器32*/ 
      {
          HAL_GPIO_WritePin(GPIOB,GPIO_PIN_10,GPIO_PIN_SET);
      }
      else                                     /*关闭继电器32*/
      {
         HAL_GPIO_WritePin(GPIOB,GPIO_PIN_10,GPIO_PIN_RESET);
       
      }        
     break;
     case 33:     
       if(tcp_recv_arr[2]==1)                  /*打开继电器33*/ 
      {
          HAL_GPIO_WritePin(GPIOB,GPIO_PIN_2,GPIO_PIN_SET);
      }
      else                                     /*关闭继电器33*/
      {
         HAL_GPIO_WritePin(GPIOB,GPIO_PIN_2,GPIO_PIN_RESET);
       
      }          
     break;
     case 34: 
       if(tcp_recv_arr[2]==1)                  /*打开继电器34*/ 
      {
          HAL_GPIO_WritePin(GPIOB,GPIO_PIN_1,GPIO_PIN_SET);
      }
      else                                     /*关闭继电器34*/
      {
         HAL_GPIO_WritePin(GPIOB,GPIO_PIN_1,GPIO_PIN_RESET);
       
      }            
     break;
     case 35:  
      if(tcp_recv_arr[2]==1)                  /*打开继电器35*/ 
      {
          HAL_GPIO_WritePin(GPIOB,GPIO_PIN_0,GPIO_PIN_SET);
      }
      else                                     /*关闭继电器35*/
      {
         HAL_GPIO_WritePin(GPIOB,GPIO_PIN_0,GPIO_PIN_RESET);
       
      }        
      break; 
     case 36:    
      if(tcp_recv_arr[2]==1)                  /*打开继电器36*/ 
      {
          HAL_GPIO_WritePin(GPIOC,GPIO_PIN_5,GPIO_PIN_SET);
      }
      else                                     /*关闭继电器36*/
      {
         HAL_GPIO_WritePin(GPIOC,GPIO_PIN_5,GPIO_PIN_RESET);
       
      }        
     break;
     case 37: 
       if(tcp_recv_arr[2]==1)                  /*打开继电器37*/ 
      {
          HAL_GPIO_WritePin(GPIOC,GPIO_PIN_4,GPIO_PIN_SET);
      }
      else                                     /*关闭继电器37*/
      {
         HAL_GPIO_WritePin(GPIOC,GPIO_PIN_4,GPIO_PIN_RESET);
       
      }          
      break;
     case 38:
       if(tcp_recv_arr[2]==1)                      /*打开继电器38*/ 
      {
          HAL_GPIO_WritePin(GPIOA,GPIO_PIN_7,GPIO_PIN_SET);
      }
      else                                          /*关闭继电器38*/
      {
         HAL_GPIO_WritePin(GPIOA,GPIO_PIN_7,GPIO_PIN_RESET);
       
      }             
      break;
     case 39: 
      if(tcp_recv_arr[2]==1)                      /*打开继电器39*/ 
      {
          HAL_GPIO_WritePin(GPIOA,GPIO_PIN_3,GPIO_PIN_SET);
      }
      else                                          /*关闭继电器39*/
      {
         HAL_GPIO_WritePin(GPIOA,GPIO_PIN_3,GPIO_PIN_RESET);
       
      }              
     break;
     case 40:    
      if(tcp_recv_arr[2]==1)                        /*打开继电器40*/ 
      {
          HAL_GPIO_WritePin(GPIOA,GPIO_PIN_5,GPIO_PIN_SET);
      }
      else                                          /*关闭继电器40*/
      {
         HAL_GPIO_WritePin(GPIOA,GPIO_PIN_5,GPIO_PIN_RESET);
       
      }        
      break;                 
      default:break;
      
     } 
   }
   
}

/**
 * @brief 回发当前网络模式确认帧
 *        格式: FE A0 mode 00 00 00 00 FF
 */
void send_net_mode_ack(void)
{
    uint8_t ack[8] = {0xFE, 0xA0, net_mode, 0x00, 0x00, 0x00, 0x00, 0xFF};
    control_transport_write(ack, 8);
}

/**
 * @brief 查询并回发当前网络配置（4帧）
 *        帧1: FE A0 mode 00 00 00 00 FF     (模式)
 *        帧2: FE A1 ip0  ip1  ip2  ip3  FF   (IP地址 4字节)
 *        帧3: FE A2 mask0 mask1 mask2 mask3 FF (子网掩码 4字节)
 *        帧4: FE A3 gw0  gw1  gw2  gw3  FF   (网关 4字节)
 */
void query_net_config(void)
{
    uint8_t buf[8];
    const ip4_addr_t *ip = netif_ip4_addr(&ch390_netif);
    const ip4_addr_t *mask = netif_ip4_netmask(&ch390_netif);
    const ip4_addr_t *gw = netif_ip4_gw(&ch390_netif);

    /* 帧1: 模式 */
    buf[0] = 0xFE; buf[1] = 0xA0; buf[2] = net_mode;
    buf[3] = 0x00; buf[4] = 0x00; buf[5] = 0x00; buf[6] = 0x00; buf[7] = 0xFF;
    control_transport_write(buf, 8);

    /* 帧2: IP地址 (4字节) */
    buf[0] = 0xFE; buf[1] = 0xA1;
    buf[2] = ip4_addr1(ip); buf[3] = ip4_addr2(ip);
    buf[4] = ip4_addr3(ip); buf[5] = ip4_addr4(ip); buf[6] = 0x00; buf[7] = 0xFF;
    control_transport_write(buf, 8);

    /* 帧3: 子网掩码 (4字节) */
    buf[0] = 0xFE; buf[1] = 0xA2;
    buf[2] = ip4_addr1(mask); buf[3] = ip4_addr2(mask);
    buf[4] = ip4_addr3(mask); buf[5] = ip4_addr4(mask); buf[6] = 0x00; buf[7] = 0xFF;
    control_transport_write(buf, 8);

    /* 帧4: 网关 (4字节) */
    buf[0] = 0xFE; buf[1] = 0xA3;
    buf[2] = ip4_addr1(gw); buf[3] = ip4_addr2(gw);
    buf[4] = ip4_addr3(gw); buf[5] = ip4_addr4(gw); buf[6] = 0x00; buf[7] = 0xFF;
    control_transport_write(buf, 8);
}

/*uart接收数据处理*/  
void uart_recv_data_process(void)   
{
  if((uart_recv_arr[0]==0xFE)&&(uart_recv_arr[7]==0xFF))
   {
     switch(uart_recv_arr[1])
     {
       case 1:                  
       if(uart_recv_arr[2]==1)   
       {                                  /*打开继电器1*/
          HAL_GPIO_WritePin(GPIOC,GPIO_PIN_13,GPIO_PIN_SET);
       }
       else                                /*关闭继电器1*/ 
       {
         HAL_GPIO_WritePin(GPIOC,GPIO_PIN_13,GPIO_PIN_RESET);
       
       }
       break;
       case 2:                          
         if(uart_recv_arr[2]==1)   
       {                                     /*打开继电器2*/
          HAL_GPIO_WritePin(GPIOC,GPIO_PIN_12,GPIO_PIN_SET);
       }
       else                                   /*关闭继电器2*/ 
       {
         HAL_GPIO_WritePin(GPIOC,GPIO_PIN_12,GPIO_PIN_RESET);
       
       }       
        break;
        case 3:                            
        if(uart_recv_arr[2]==1)        
       {                                     /*打开继电器3*/ 
          HAL_GPIO_WritePin(GPIOC,GPIO_PIN_11,GPIO_PIN_SET);
       }
       else                                    /*关闭继电器3*/ 
       {
         HAL_GPIO_WritePin(GPIOC,GPIO_PIN_11,GPIO_PIN_RESET);
       
       }            
        break;
        case 4:                              
        if(uart_recv_arr[2]==1)        
       {                                       /*打开继电器4*/ 
          HAL_GPIO_WritePin(GPIOC,GPIO_PIN_10,GPIO_PIN_SET);
       }
       else                                /*关闭继电器4*/ 
       {
         HAL_GPIO_WritePin(GPIOC,GPIO_PIN_10,GPIO_PIN_RESET);
       
       }         
        break;
       case 5:    
        if(uart_recv_arr[2]==1)               /*打开继电器5*/ 
       {
          HAL_GPIO_WritePin(GPIOB,GPIO_PIN_9,GPIO_PIN_SET);
       }
       else                                   /*关闭继电器5*/
       {
         HAL_GPIO_WritePin(GPIOB,GPIO_PIN_9,GPIO_PIN_RESET);
       
       }              
        break; 
        case 6:      
        if(uart_recv_arr[2]==1)               /*打开继电器6*/ 
       {
          HAL_GPIO_WritePin(GPIOB,GPIO_PIN_8,GPIO_PIN_SET);
       }
       else                                   /*关闭继电器6*/
       {
         HAL_GPIO_WritePin(GPIOB,GPIO_PIN_8,GPIO_PIN_RESET);
       
       }             
        break;
        case 7:    
       if(uart_recv_arr[2]==1)               /*打开继电器7*/ 
       {
          HAL_GPIO_WritePin(GPIOB,GPIO_PIN_5,GPIO_PIN_SET);
       }
       else                                   /*关闭继电器7*/
       {
         HAL_GPIO_WritePin(GPIOB,GPIO_PIN_5,GPIO_PIN_RESET);
       
       }     
        break;
        case 8:     
        if(uart_recv_arr[2]==1)               /*打开继电器8*/ 
       {
          HAL_GPIO_WritePin(GPIOB,GPIO_PIN_4,GPIO_PIN_SET);
       }
       else                                   /*关闭继电器8*/
       {
         HAL_GPIO_WritePin(GPIOB,GPIO_PIN_4,GPIO_PIN_RESET);
       
       }     
        break;
        case 9:      
         if(uart_recv_arr[2]==1)               /*打开继电器9*/ 
       {
          HAL_GPIO_WritePin(GPIOB,GPIO_PIN_3,GPIO_PIN_SET);
       }
       else                                    /*关闭继电器9*/
       {
         HAL_GPIO_WritePin(GPIOB,GPIO_PIN_3,GPIO_PIN_RESET);
       
       }                  
        break;
        case 10:    
       if(uart_recv_arr[2]==1)               /*打开继电器10*/ 
       {
          HAL_GPIO_WritePin(GPIOD,GPIO_PIN_4,GPIO_PIN_SET);
       }
       else                                    /*关闭继电器10*/
       {
         HAL_GPIO_WritePin(GPIOD,GPIO_PIN_4,GPIO_PIN_RESET);
       
       }       
        break; 
        case 11:    
      if(uart_recv_arr[2]==1)                /*打开继电器11*/ 
       {
          HAL_GPIO_WritePin(GPIOD,GPIO_PIN_3,GPIO_PIN_SET);
       }
       else                                    /*关闭继电器11*/
       {
         HAL_GPIO_WritePin(GPIOD,GPIO_PIN_3,GPIO_PIN_RESET);
       
       }       
        break;
        case 12:     
       if(uart_recv_arr[2]==1)                /*打开继电器12*/ 
       {
          HAL_GPIO_WritePin(GPIOD,GPIO_PIN_2,GPIO_PIN_SET);
       }
       else                                    /*关闭继电器12*/
       {
         HAL_GPIO_WritePin(GPIOD,GPIO_PIN_2,GPIO_PIN_RESET);
       
       }           
        break;
        case 13: 
         if(uart_recv_arr[2]==1)                /*打开继电器13*/ 
       {
          HAL_GPIO_WritePin(GPIOD,GPIO_PIN_1,GPIO_PIN_SET);
       }
       else                                    /*关闭继电器13*/
       {
         HAL_GPIO_WritePin(GPIOD,GPIO_PIN_1,GPIO_PIN_RESET);
       
       }             
        break;
        case 14: 
        if(uart_recv_arr[2]==1)                /*打开继电器14*/ 
       {
          HAL_GPIO_WritePin(GPIOD,GPIO_PIN_0,GPIO_PIN_SET);
       }
       else                                    /*关闭继电器14*/
       {
         HAL_GPIO_WritePin(GPIOD,GPIO_PIN_0,GPIO_PIN_RESET);
       
       }            
        break;
        case 15:  
       if(uart_recv_arr[2]==1)                /*打开继电器15*/ 
       {
          HAL_GPIO_WritePin(GPIOC,GPIO_PIN_9,GPIO_PIN_SET);
       }
       else                                    /*关闭继电器15*/
       {
         HAL_GPIO_WritePin(GPIOC,GPIO_PIN_9,GPIO_PIN_RESET);
       
       }           
        break; 
        case 16:  
       if(uart_recv_arr[2]==1)                /*打开继电器16*/ 
       {
          HAL_GPIO_WritePin(GPIOC,GPIO_PIN_8,GPIO_PIN_SET);
       }
       else                                    /*关闭继电器16*/
       {
         HAL_GPIO_WritePin(GPIOC,GPIO_PIN_8,GPIO_PIN_RESET);
       
       }        
      break;
     case 17:     
       if(uart_recv_arr[2]==1)                /*打开继电器17*/ 
       {
          HAL_GPIO_WritePin(GPIOA,GPIO_PIN_15,GPIO_PIN_SET);
       }
       else                                    /*关闭继电器17*/
       {
         HAL_GPIO_WritePin(GPIOA,GPIO_PIN_15,GPIO_PIN_RESET);
       
       }          
     break;
     case 18:
      if(uart_recv_arr[2]==1)                  /*打开继电器18*/ 
       {
          HAL_GPIO_WritePin(GPIOA,GPIO_PIN_12,GPIO_PIN_SET);
       }
       else                                    /*关闭继电器18*/
       {
         HAL_GPIO_WritePin(GPIOA,GPIO_PIN_12,GPIO_PIN_RESET);
       
       }                
     break;
     case 19: 
      if(uart_recv_arr[2]==1)                  /*打开继电器19*/ 
       {
          HAL_GPIO_WritePin(GPIOA,GPIO_PIN_11,GPIO_PIN_SET);
       }
       else                                    /*关闭继电器19*/
       {
         HAL_GPIO_WritePin(GPIOA,GPIO_PIN_11,GPIO_PIN_RESET);
       
       }       
     break;
     case 20: 
     if(uart_recv_arr[2]==1)                  /*打开继电器20*/ 
       {
          HAL_GPIO_WritePin(GPIOA,GPIO_PIN_10,GPIO_PIN_SET);
       }
       else                                    /*关闭继电器20*/
       {
         HAL_GPIO_WritePin(GPIOA,GPIO_PIN_10,GPIO_PIN_RESET);
       
       }        
     break; 
     case 21:    
     if(uart_recv_arr[2]==1)                  /*打开继电器21*/ 
       {
          HAL_GPIO_WritePin(GPIOD,GPIO_PIN_9,GPIO_PIN_SET);
       }
       else                                    /*关闭继电器21*/
       {
         HAL_GPIO_WritePin(GPIOD,GPIO_PIN_9,GPIO_PIN_RESET);
       
       }       
     break;
     case 22:    
      if(uart_recv_arr[2]==1)                  /*打开继电器22*/ 
       {
          HAL_GPIO_WritePin(GPIOD,GPIO_PIN_8,GPIO_PIN_SET);
       }
       else                                    /*关闭继电器22*/
       {
         HAL_GPIO_WritePin(GPIOD,GPIO_PIN_8,GPIO_PIN_RESET);
       
       }         
     break;
     case 23:  
      if(uart_recv_arr[2]==1)                  /*打开继电器23*/ 
       {
          HAL_GPIO_WritePin(GPIOC,GPIO_PIN_7,GPIO_PIN_SET);
       }
       else                                    /*关闭继电器23*/
       {
         HAL_GPIO_WritePin(GPIOC,GPIO_PIN_7,GPIO_PIN_RESET);
       
       }       
     break;
     case 24:     
       if(uart_recv_arr[2]==1)                  /*打开继电器24*/ 
      {
          HAL_GPIO_WritePin(GPIOC,GPIO_PIN_6,GPIO_PIN_SET);
      }
      else                                    /*关闭继电器24*/
      {
         HAL_GPIO_WritePin(GPIOC,GPIO_PIN_6,GPIO_PIN_RESET);
       
      }           
     break;
     case 25:  
      if(uart_recv_arr[2]==1)                  /*打开继电器25*/ 
      {
          HAL_GPIO_WritePin(GPIOA,GPIO_PIN_9,GPIO_PIN_SET);
      }
      else                                    /*关闭继电器25*/
      {
         HAL_GPIO_WritePin(GPIOA,GPIO_PIN_9,GPIO_PIN_RESET);
       
      }            
     break; 
     case 26: 
      if(uart_recv_arr[2]==1)                  /*打开继电器26*/ 
      {
          HAL_GPIO_WritePin(GPIOA,GPIO_PIN_8,GPIO_PIN_SET);
      }
      else                                    /*关闭继电器26*/
      {
         HAL_GPIO_WritePin(GPIOA,GPIO_PIN_8,GPIO_PIN_RESET);
       
      }           
      break;
     case 27:   
       if(uart_recv_arr[2]==1)                  /*打开继电器27*/ 
      {
          HAL_GPIO_WritePin(GPIOB,GPIO_PIN_15,GPIO_PIN_SET);
      }
      else                                    /*关闭继电器27*/
      {
         HAL_GPIO_WritePin(GPIOB,GPIO_PIN_15,GPIO_PIN_RESET);
       
      }         
      break;
     case 28: 
       if(uart_recv_arr[2]==1)                  /*打开继电器28*/ 
      {
          HAL_GPIO_WritePin(GPIOB,GPIO_PIN_14,GPIO_PIN_SET);
      }
      else                                    /*关闭继电器28*/
      {
         HAL_GPIO_WritePin(GPIOB,GPIO_PIN_14,GPIO_PIN_RESET);
       
      }    
     break;
     case 29:   
        if(uart_recv_arr[2]==1)                  /*打开继电器29*/ 
      {
          HAL_GPIO_WritePin(GPIOB,GPIO_PIN_13,GPIO_PIN_SET);
      }
      else                                     /*关闭继电器29*/
      {
         HAL_GPIO_WritePin(GPIOB,GPIO_PIN_13,GPIO_PIN_RESET);
       
      }    
     break;
     case 30:     
        if(uart_recv_arr[2]==1)                  /*打开继电器30*/ 
      {
          HAL_GPIO_WritePin(GPIOB,GPIO_PIN_12,GPIO_PIN_SET);
      }
      else                                     /*关闭继电器30*/
      {
         HAL_GPIO_WritePin(GPIOB,GPIO_PIN_12,GPIO_PIN_RESET);
       
      }      
     break;
     case 31:     
         if(uart_recv_arr[2]==1)                  /*打开继电器31*/ 
      {
          HAL_GPIO_WritePin(GPIOB,GPIO_PIN_11,GPIO_PIN_SET);
      }
      else                                     /*关闭继电器31*/
      {
         HAL_GPIO_WritePin(GPIOB,GPIO_PIN_11,GPIO_PIN_RESET);
       
      }        
     break;
     case 32:     
         if(uart_recv_arr[2]==1)                  /*打开继电器32*/ 
      {
          HAL_GPIO_WritePin(GPIOB,GPIO_PIN_10,GPIO_PIN_SET);
      }
      else                                     /*关闭继电器32*/
      {
         HAL_GPIO_WritePin(GPIOB,GPIO_PIN_10,GPIO_PIN_RESET);
       
      }        
     break;
     case 33:     
       if(uart_recv_arr[2]==1)                  /*打开继电器33*/ 
      {
          HAL_GPIO_WritePin(GPIOB,GPIO_PIN_2,GPIO_PIN_SET);
      }
      else                                     /*关闭继电器33*/
      {
         HAL_GPIO_WritePin(GPIOB,GPIO_PIN_2,GPIO_PIN_RESET);
       
      }          
     break;
     case 34: 
       if(uart_recv_arr[2]==1)                  /*打开继电器34*/ 
      {
          HAL_GPIO_WritePin(GPIOB,GPIO_PIN_1,GPIO_PIN_SET);
      }
      else                                     /*关闭继电器34*/
      {
         HAL_GPIO_WritePin(GPIOB,GPIO_PIN_1,GPIO_PIN_RESET);
       
      }            
     break;
     case 35:  
      if(uart_recv_arr[2]==1)                  /*打开继电器35*/ 
      {
          HAL_GPIO_WritePin(GPIOB,GPIO_PIN_0,GPIO_PIN_SET);
      }
      else                                     /*关闭继电器35*/
      {
         HAL_GPIO_WritePin(GPIOB,GPIO_PIN_0,GPIO_PIN_RESET);
       
      }        
      break; 
     case 36:    
      if(uart_recv_arr[2]==1)                  /*打开继电器36*/ 
      {
          HAL_GPIO_WritePin(GPIOC,GPIO_PIN_5,GPIO_PIN_SET);
      }
      else                                     /*关闭继电器36*/
      {
         HAL_GPIO_WritePin(GPIOC,GPIO_PIN_5,GPIO_PIN_RESET);
       
      }        
     break;
     case 37: 
       if(uart_recv_arr[2]==1)                  /*打开继电器37*/ 
      {
          HAL_GPIO_WritePin(GPIOC,GPIO_PIN_4,GPIO_PIN_SET);
      }
      else                                     /*关闭继电器37*/
      {
         HAL_GPIO_WritePin(GPIOC,GPIO_PIN_4,GPIO_PIN_RESET);
       
      }          
      break;
     case 38:
       if(uart_recv_arr[2]==1)                      /*打开继电器38*/ 
      {
          HAL_GPIO_WritePin(GPIOA,GPIO_PIN_7,GPIO_PIN_SET);
      }
      else                                          /*关闭继电器38*/
      {
         HAL_GPIO_WritePin(GPIOA,GPIO_PIN_7,GPIO_PIN_RESET);
       
      }             
      break;
     case 39: 
      if(uart_recv_arr[2]==1)                      /*打开继电器39*/ 
      {
          HAL_GPIO_WritePin(GPIOA,GPIO_PIN_3,GPIO_PIN_SET);
      }
      else                                          /*关闭继电器39*/
      {
         HAL_GPIO_WritePin(GPIOA,GPIO_PIN_3,GPIO_PIN_RESET);
       
      }              
     break;
     case 40:    
      if(uart_recv_arr[2]==1)                        /*打开继电器40*/ 
      {
          HAL_GPIO_WritePin(GPIOA,GPIO_PIN_5,GPIO_PIN_SET);
      }
      else                                          /*关闭继电器40*/
      {
         HAL_GPIO_WritePin(GPIOA,GPIO_PIN_5,GPIO_PIN_RESET);
       
      }
      break;
      /* ===== 网络配置命令 ===== */
      case 0xA0:  /* 设置网络模式: 0=DHCP, 1=静态IP */
      {
        uint8_t mode = uart_recv_arr[2];
        if (mode == 0)  /* 切换到 DHCP */
        {
          dhcp_stop(&ch390_netif);
          ip4_addr_t zero;
          IP4_ADDR(&zero, 0, 0, 0, 0);
          netif_set_addr(&ch390_netif, &zero, &zero, &zero);
          dhcp_start(&ch390_netif);
          net_mode = 0;
          static_ip_frame = 0;
          flash_net_cfg_clear();  /* 清除Flash中的静态配置 */
        }
        else if (mode == 1)  /* 切换到静态IP (等待后续帧配置地址) */
        {
          net_mode = 1;
          static_ip_frame = 0;  /* 重置帧计数，等待 0xA1~0xA3 */
        }
        send_net_mode_ack();
      }
      break;
      case 0xA1:  /* 静态IP: ip0, ip1, ip2, ip3 (4字节完整IP) */
      {
        if (net_mode == 1)
        {
          static_ip[0] = uart_recv_arr[2];
          static_ip[1] = uart_recv_arr[3];
          static_ip[2] = uart_recv_arr[4];
          static_ip[3] = uart_recv_arr[5];
          static_ip_frame = 1;
        }
      }
      break;
      case 0xA2:  /* 子网掩码: mask0, mask1, mask2, mask3 (4字节完整掩码) */
      {
        if (net_mode == 1 && static_ip_frame >= 1)
        {
          static_mask[0] = uart_recv_arr[2];
          static_mask[1] = uart_recv_arr[3];
          static_mask[2] = uart_recv_arr[4];
          static_mask[3] = uart_recv_arr[5];
          static_ip_frame = 2;
        }
      }
      break;
      case 0xA3:  /* 网关: gw0, gw1, gw2, gw3 (4字节完整网关) -> 应用配置 */
      {
        if (net_mode == 1 && static_ip_frame >= 2)
        {
          static_gw[0] = uart_recv_arr[2];
          static_gw[1] = uart_recv_arr[3];
          static_gw[2] = uart_recv_arr[4];
          static_gw[3] = uart_recv_arr[5];
          /* 应用静态IP配置 */
          dhcp_stop(&ch390_netif);
          ip4_addr_t ip, mask, gw;
          IP4_ADDR(&ip, static_ip[0], static_ip[1], static_ip[2], static_ip[3]);
          IP4_ADDR(&mask, static_mask[0], static_mask[1], static_mask[2], static_mask[3]);
          IP4_ADDR(&gw, static_gw[0], static_gw[1], static_gw[2], static_gw[3]);
          netif_set_addr(&ch390_netif, &ip, &mask, &gw);
          netif_set_up(&ch390_netif);
          static_ip_frame = 0;
          /* 保存到Flash */
          FlashNetCfg_t save_cfg;
          save_cfg.net_mode = 1;
          memcpy(save_cfg.ip, static_ip, 4);
          memcpy(save_cfg.mask, static_mask, 4);
          memcpy(save_cfg.gw, static_gw, 4);
          memset(save_cfg.reserved, 0, 3);
          flash_net_cfg_save(&save_cfg);
          /* 回发确认 */
          send_net_mode_ack();
          // printf("Static IP set: %s\r\n", ip4addr_ntoa(&ip));
        }
      }
      break;
      case 0xA4:  /* 查询当前网络配置 */
      {
        query_net_config();
      }
      break;        
      default: break;
      
     } 
   }
   
}

