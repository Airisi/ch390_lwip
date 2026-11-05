#include "flash_net_cfg.h"
#include "main.h"
#include <string.h>

/**
 * @brief 计算简单校验和 (对前20字节做累加)
 */
static uint32_t flash_net_cfg_calc_checksum(const FlashNetCfg_t *cfg)
{
    uint32_t sum = 0;
    const uint8_t *p = (const uint8_t *)cfg;
    /* 跳过checksum字段本身(最后4字节)，对前20字节求和 */
    for (uint32_t i = 0; i < offsetof(FlashNetCfg_t, checksum); i++)
    {
        sum += p[i];
    }
    return sum;
}

uint8_t flash_net_cfg_load(FlashNetCfg_t *cfg)
{
    /* 直接读取Flash地址，强转指针 */
    const FlashNetCfg_t *flash_cfg = (const FlashNetCfg_t *)FLASH_NET_CFG_ADDR;

    /* 检查魔数 */
    if (flash_cfg->magic != FLASH_NET_CFG_MAGIC)
    {
        return 1;  /* Flash中无有效数据 */
    }

    /* 校验和验证 */
    uint32_t expected = flash_net_cfg_calc_checksum(flash_cfg);
    if (expected != flash_cfg->checksum)
    {
        return 1;  /* 数据损坏 */
    }

    /* 拷贝到输出 */
    memcpy(cfg, flash_cfg, sizeof(FlashNetCfg_t));
    return 0;
}

uint8_t flash_net_cfg_save(FlashNetCfg_t *cfg)
{
    HAL_StatusTypeDef status;

    /* 计算校验和 */
    cfg->magic = FLASH_NET_CFG_MAGIC;
    cfg->checksum = flash_net_cfg_calc_checksum(cfg);

    /* 解锁Flash */
    HAL_FLASH_Unlock();

    /* 擦除参数页 (F072 地址方式) */
    FLASH_EraseInitTypeDef erase;
    uint32_t page_error = 0;
    erase.TypeErase = FLASH_TYPEERASE_PAGES;
    erase.PageAddress = FLASH_NET_CFG_ADDR;                        /* 128KB Flash的最后一页 */
    erase.NbPages = 1;

    status = HAL_FLASHEx_Erase(&erase, &page_error);
    if (status != HAL_OK)
    {
        HAL_FLASH_Lock();
        return 1;
    }

    /* 逐2字节写入 (FLASH_TYPEPROGRAM_HALFWORD) */
    uint16_t *src = (uint16_t *)cfg;
    uint32_t addr = FLASH_NET_CFG_ADDR;
    uint32_t words = sizeof(FlashNetCfg_t) / 2;  /* 24/2 = 12次 */

    for (uint32_t i = 0; i < words; i++)
    {
        status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, addr, src[i]);
        if (status != HAL_OK)
        {
            HAL_FLASH_Lock();
            return 1;
        }
        addr += 2;
    }

    /* 处理剩余字节（如果有，24字节恰好是12个uint16_t，无需额外处理） */

    /* 锁定Flash */
    HAL_FLASH_Lock();
    return 0;
}

uint8_t flash_net_cfg_clear(void)
{
    HAL_StatusTypeDef status;

    HAL_FLASH_Unlock();

    FLASH_EraseInitTypeDef erase;
    uint32_t page_error = 0;
    erase.TypeErase = FLASH_TYPEERASE_PAGES;
    erase.PageAddress = FLASH_NET_CFG_ADDR;
    erase.NbPages = 1;

    status = HAL_FLASHEx_Erase(&erase, &page_error);

    HAL_FLASH_Lock();
    return (status == HAL_OK) ? 0 : 1;
}
