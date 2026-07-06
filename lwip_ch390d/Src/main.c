/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "rtc.h"
#include "spi.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "delay.h"
#include "CH390.h"
#include "CH390_Interface.h"

#include "netif/ethernetif.h"
#include "lwip/timeouts.h"
#include "lwip/dhcp.h"
#include "lwip/prot/dhcp.h"
#include "tcp_echo.h"
#include "udp_echo.h"
#include "lwip/udp.h"
#include <string.h>
#include "lwipopts.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
#define USE_DHCP 1
extern uint8_t tcp_recv_arr[8];
extern uint8_t uart_recv_flag;
extern UART_HandleTypeDef huart1;
extern uint8_t uart_recv_arr[8];
extern uint8_t tcp_recv_flag;
extern uint8_t udp_flag;
void tcp_recv_data_process(void); 
void uart_recv_data_process(void); 

//extern uint8_t  tcp_recv_flag;
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
  char reply[64]={0};

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
/**
 * @name ch390_int_handler
 * @brief Handle CH390 interrupt events, include packet receive
 */
void ch390_int_handler()
{
    uint8_t int_status = ch390_get_int_status();
    // Link status changed
    if(int_status & ISR_LNKCHG)
    {
       HAL_Delay (65);
      if (ch390_get_link_status())
       {
         // printf("netif link up\r\n");
            netif_set_link_up(&ch390_netif);
        }
        else
        {
          //  printf("netif link down\r\n");
            netif_set_link_down(&ch390_netif);
        }
        ch390_write_reg(CH390_ISR, ISR_LNKCHG);
    }
    // Receive overflow
      if(int_status & ISR_ROS)
     {
      //  printf("Receive overflow\r\n");
        struct ethernetif *ethernetif = ch390_netif.state;
        do {
            ethernetif_input(&ch390_netif);
        } while (ethernetif->rx_len != 0);
     }
    // Receive overflow counter overflow
    if (int_status & ISR_ROO) printf("Overflow counter overflow\r\n");
    // Packet transmitted
    //    if(int_status & ISR_PT) printf("Packet sent\r\n");
    // Packet received
    if (int_status & ISR_PR)
    {
        struct ethernetif *ethernetif = ch390_netif.state;
        do {
           ethernetif_input(&ch390_netif);
        } while (ethernetif->rx_len != 0);
    }
}
/**
 * @name ch390_print_info
 * @brief Read and print chip's ID and MAC address
 */
void ch390_print_info()
{
    uint8_t i;
    uint8_t mac_addr[6];
    uint16_t vid = ch390_get_vendor_id();
    uint16_t pid = ch390_get_product_id();
    printf("ID: %04x%04x\r\n", vid, pid);

    ch390_get_mac(mac_addr);
    printf("MAC: ");
    for (i = 0; i < 6; i++)
    {
        printf("%02X ", mac_addr[i]);
    }
    printf("\r\n");
}
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */
  struct ip4_addr ipaddr, netmask, gateway;
  struct dhcp *dhcp;
  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();
  HAL_Delay(500);
  /* USER CODE BEGIN Init */
  
  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */
//  #ifdef CH390_INTERFACE_SPI
//   ch390_gpio_init ();
//  #endif
  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_RTC_Init();
  MX_SPI1_Init();
  MX_USART1_UART_Init();
  HAL_Delay(10);
  /* USER CODE BEGIN 2 */
  IP4_ADDR(&ipaddr, 0, 0, 0, 0);
  IP4_ADDR(&netmask, 0, 0, 0, 0);
  IP4_ADDR(&gateway, 0, 0, 0, 0);
  init_lwip_netif(&ipaddr, &netmask, &gateway);
    // Wait until phy link up
//   while (!ch390_get_link_status())
//   {
//        HAL_Delay(100);
//   }
//  netif_set_link_up(&ch390_netif);
//  printf("netif link up\r\n");

#if USE_DHCP
    dhcp_start(&ch390_netif);
//    struct dhcp *dhcp;
//    dhcp = netif_dhcp_data(&ch390_netif);
//    // Wait untill dhcp complete
//    while (dhcp->state != DHCP_STATE_BOUND)
//    {
//        if (ch390_get_int_pin())
//        {
//            ch390_int_handler();
//        }
//        sys_check_timeouts();
//    }
//     printf("DHCP complete\r\n");
//     printf("IP: %s\r\n", ip4addr_ntoa(netif_ip4_addr(&ch390_netif)));
//    
    
//    printf("DHCP complete\r\n");
//    printf("IP: %s\r\n", ip4addr_ntoa(netif_ip4_addr(&ch390_netif)));
#endif 
   udpecho_init();
   tcp_client_init();
   tcp_server_init();
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */

  while (1)
  {
    if (ch390_get_int_pin())
     { 
       ch390_int_handler();      
      if((tcp_recv_arr[0]!=0)&&( tcp_recv_flag==1))
      {
        tcp_recv_flag=0;        
        tcp_recv_data_process(); 
        memset(tcp_recv_arr,0,sizeof(tcp_recv_arr));       
       }
     }
     sys_check_timeouts ();
    /* USER CODE END WHILE */
    /* USER CODE BEGIN 3 */
     if(uart_recv_flag==1)
     {
       uart_recv_flag=0;
       uart_recv_data_process();         
     } 
     dhcp = netif_dhcp_data(&ch390_netif);
    // Wait untill dhcp complete
    if (dhcp->state != DHCP_STATE_BOUND)
    {
       if (ch390_get_int_pin())
        {
            ch390_int_handler();
        }
        sys_check_timeouts ();
    }    
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Configure the main internal regulator output voltage
  */
  HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1);
  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSI|RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.LSIState = RCC_LSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = RCC_PLLM_DIV1;
  RCC_OscInitStruct.PLL.PLLN = 16;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the peripherals clocks
  */
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_RTC|RCC_PERIPHCLK_USART1;
  PeriphClkInit.Usart1ClockSelection = RCC_USART1CLKSOURCE_PCLK1;
  PeriphClkInit.RTCClockSelection = RCC_RTCCLKSOURCE_LSI;

  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

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
      default: break;
      
     } 
   }
   
}

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
