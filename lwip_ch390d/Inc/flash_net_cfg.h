#ifndef __FLASH_NET_CFG_H
#define __FLASH_NET_CFG_H

#include "stm32g0xx_hal.h"

/* Flash最后一页起始地址 (128KB Flash, 页63) */
#define FLASH_NET_CFG_ADDR      0x0801F800U

/* 魔数校验，用于判断Flash中是否存有有效配置 */
#define FLASH_NET_CFG_MAGIC     0xA55A5AA5U

/**
 * @brief 网络配置存储结构体 (24字节)
 */
typedef struct {
    uint32_t magic;         /* 魔数校验 */
    uint8_t  net_mode;      /* 0=DHCP, 1=静态IP */
    uint8_t  ip[4];         /* 静态IP地址 */
    uint8_t  mask[4];       /* 子网掩码 */
    uint8_t  gw[4];         /* 网关 */
    uint8_t  reserved[3];   /* 保留对齐 */
    uint32_t checksum;      /* 校验和 */
} FlashNetCfg_t;

/**
 * @brief  从Flash加载网络配置
 * @param  cfg: 输出配置数据
 * @retval 0=成功(有效数据), 1=Flash无有效数据
 */
uint8_t flash_net_cfg_load(FlashNetCfg_t *cfg);

/**
 * @brief  将网络配置保存到Flash
 * @param  cfg: 待保存的配置数据
 * @retval 0=成功, 1=写入失败
 */
uint8_t flash_net_cfg_save(FlashNetCfg_t *cfg);

/**
 * @brief  清除Flash中的网络配置
 * @retval 0=成功, 1=擦除失败
 */
uint8_t flash_net_cfg_clear(void);

#endif /* __FLASH_NET_CFG_H */
