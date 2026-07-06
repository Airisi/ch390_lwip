#include "delay.h"


/**
  * @brief Systick微秒延时
  * @note 使用Systick的24位计数器
  * @param us: 微秒数
  * @retval None
  */
void delay_us(uint32_t us)
{
    uint32_t start_tick = SysTick->VAL;
    uint32_t ticks_per_us = SystemCoreClock / 1000000;
    uint32_t delay_ticks = us * ticks_per_us;
    uint32_t end_tick;
    
    if (delay_ticks > SysTick->LOAD)
    {
        // 延时时间超过Systick重载值
        uint32_t full_cycles = delay_ticks / SysTick->LOAD;
        uint32_t remainder = delay_ticks % SysTick->LOAD;
        
        for(uint32_t i = 0; i < full_cycles; i++)
        {
            while(SysTick->VAL < start_tick);
            while(SysTick->VAL > start_tick);
        }
        
        end_tick = (start_tick >= remainder) ? (start_tick - remainder) : (SysTick->LOAD - (remainder - start_tick));
        while(((SysTick->VAL > end_tick) && (SysTick->VAL <= start_tick)) || 
              ((SysTick->VAL <= end_tick) && (SysTick->VAL > start_tick)));
    }
    else
    {
        end_tick = (start_tick >= delay_ticks) ? (start_tick - delay_ticks) : (SysTick->LOAD - (delay_ticks - start_tick));
        while(((SysTick->VAL > end_tick) && (SysTick->VAL <= start_tick)) || 
              ((SysTick->VAL <= end_tick) && (SysTick->VAL > start_tick)));
    }
}




