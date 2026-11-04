#include "delay.h"
#include "main.h"

void delay_us(uint32_t us)
{
    uint32_t previous = SysTick->VAL;
    uint32_t period = SysTick->LOAD + 1U;
    uint32_t remaining = us * (SystemCoreClock / 1000000U);
    while (remaining != 0U) {
        uint32_t current = SysTick->VAL;
        uint32_t elapsed = previous >= current ? previous - current
                                               : previous + period - current;
        if (elapsed >= remaining) {
            break;
        }
        remaining -= elapsed;
        previous = current;
    }
}
