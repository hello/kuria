/**
 * @file
 *
 * @brief System level hardware functionality
 *
 */

#include "asf.h"
#include "board.h"
#include "xep_hal.h"

#define RESET_REASON_GPBR_REG   4
#define RESET_COUNT_GPBR_REG    5

int xt_wait(uint32_t ms_to_wait)
{
    vTaskDelay(ms_to_wait / portTICK_PERIOD_MS);

    return XT_SUCCESS;
}

int xt_software_reset(xt_swreset_reason_t reason)
{
    xt_set_reset_reason(reason);

    rstc_start_software_reset(RSTC);

    // Reset of MCU should be triggered prior to this,
    // any return from this function is an error.
    return XT_ERROR;
}

int xt_set_reset_reason(xt_swreset_reason_t reason)
{
    gpbr_write(RESET_REASON_GPBR_REG, reason);

    return XT_SUCCESS;
}

int xt_get_reset_reason(xt_swreset_reason_t * reason)
{
    static bool reset_reason_is_read = false;
    static xt_swreset_reason_t reset_reason = XT_SWRST_NONE;

    if (!reset_reason_is_read)
    {
        reset_reason = gpbr_read(RESET_REASON_GPBR_REG);
        xt_set_reset_reason(XT_SWRST_NONE);

        reset_reason_is_read = true;
    }

    *reason = reset_reason;

    return XT_SUCCESS;
}

int xt_feed_watchdog(void)
{
    wdt_restart(WDT);
	return XT_SUCCESS;
}

xt_opmode_t xt_get_operation_mode(void)
{
    static xt_opmode_t operation_mode = XT_OPMODE_UNINITIALIZED;

    if (operation_mode == XT_OPMODE_UNINITIALIZED)
    {
        pio_configure_pin(XPIN_IO_SEL1, PIO_TYPE_PIO_INPUT | PIO_PULLUP);
        pio_configure_pin(XPIN_IO_SEL2, PIO_TYPE_PIO_INPUT | PIO_PULLUP);
        operation_mode = (ioport_get_pin_level(XPIN_IO_SEL1)<<0) | (ioport_get_pin_level(XPIN_IO_SEL2)<<1);
    }
   
	return operation_mode;
}