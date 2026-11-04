#ifndef F072_FLASH_NET_CFG_H
#define F072_FLASH_NET_CFG_H
#include <stddef.h>
#include <stdint.h>
#ifndef FLASH_NET_CFG_ADDR
#define FLASH_NET_CFG_ADDR  0x0801F800U
#endif
#define FLASH_NET_CFG_MAGIC 0xA55A5AA5U
typedef struct {
    uint32_t magic;
    uint8_t net_mode;
    uint8_t ip[4];
    uint8_t mask[4];
    uint8_t gw[4];
    uint8_t reserved[3];
    uint32_t checksum;
} FlashNetCfg_t;
uint8_t flash_net_cfg_load(FlashNetCfg_t *cfg);
uint8_t flash_net_cfg_save(FlashNetCfg_t *cfg);
uint8_t flash_net_cfg_clear(void);
#endif
