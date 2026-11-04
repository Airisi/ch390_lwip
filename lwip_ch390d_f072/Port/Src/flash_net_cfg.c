#include "flash_net_cfg.h"
#include "main.h"
#include <string.h>

static uint32_t checksum(const FlashNetCfg_t *cfg)
{
    const uint8_t *bytes = (const uint8_t *)cfg;
    uint32_t sum = 0;
    size_t i;
    for (i = 0; i < offsetof(FlashNetCfg_t, checksum); ++i) { sum += bytes[i]; }
    return sum;
}

uint8_t flash_net_cfg_load(FlashNetCfg_t *cfg)
{
    const FlashNetCfg_t *saved = (const FlashNetCfg_t *)FLASH_NET_CFG_ADDR;
    if (cfg == NULL || saved->magic != FLASH_NET_CFG_MAGIC ||
        saved->net_mode != 1 || saved->checksum != checksum(saved)) {
        return 1;
    }
    memcpy(cfg, saved, sizeof(*cfg));
    return 0;
}

static HAL_StatusTypeDef erase_page(void)
{
    FLASH_EraseInitTypeDef erase = {0};
    uint32_t error;
    erase.TypeErase = FLASH_TYPEERASE_PAGES;
    erase.PageAddress = FLASH_NET_CFG_ADDR;
    erase.NbPages = 1;
    return HAL_FLASHEx_Erase(&erase, &error);
}

uint8_t flash_net_cfg_save(FlashNetCfg_t *cfg)
{
    HAL_StatusTypeDef status;
    uintptr_t address = FLASH_NET_CFG_ADDR;
    size_t offset;
    if (cfg == NULL) { return 1; }
    cfg->magic = FLASH_NET_CFG_MAGIC;
    cfg->checksum = checksum(cfg);
    if (HAL_FLASH_Unlock() != HAL_OK) { return 1; }
    status = erase_page();
    /* Commit the magic last: a partially programmed record remains invalid. */
    for (offset = sizeof(cfg->magic); status == HAL_OK && offset < sizeof(*cfg); offset += 2) {
        uint16_t halfword;
        memcpy(&halfword, (const uint8_t *)cfg + offset, sizeof(halfword));
        status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, address + offset, halfword);
    }
    for (offset = 0; status == HAL_OK && offset < sizeof(cfg->magic); offset += 2) {
        uint16_t halfword;
        memcpy(&halfword, (const uint8_t *)cfg + offset, sizeof(halfword));
        status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, address + offset, halfword);
    }
    HAL_FLASH_Lock();
    return status != HAL_OK || memcmp((const void *)address, cfg, sizeof(*cfg)) != 0;
}

uint8_t flash_net_cfg_clear(void)
{
    HAL_StatusTypeDef status;
    if (HAL_FLASH_Unlock() != HAL_OK) { return 1; }
    status = erase_page();
    HAL_FLASH_Lock();
    return status != HAL_OK;
}
