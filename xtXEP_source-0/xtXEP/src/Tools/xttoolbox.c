/**
 * @file
 *
 * 
 */

#include "xttoolbox.h"
#include "asf.h"
#include <FreeRTOS.h>
#include "task.h"

uint64_t xttb_tick_os(void)
{
    uint64_t ostick = xTaskGetTickCount();

    return ostick;
}

uint64_t xttb_systimer_us(void)
{
    uint64_t ostick = xTaskGetTickCount();
    uint64_t period = SysTick->LOAD-1;
    uint64_t systickval = SysTick->VAL;
    uint64_t systick_us = systickval * 1000 / period;
    uint64_t us = ostick * 1000 + systick_us;

    return us;
}
