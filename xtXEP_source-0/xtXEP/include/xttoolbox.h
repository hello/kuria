/**
 * @file
 * @brief Misc tools.
 *
 */

#ifndef XTTOOLBOX_H
#define XTTOOLBOX_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Get system os time tick.
 *
 * @return Tick value.
 */
uint64_t xttb_tick_os(void);

/**
 * @brief Get accurate timer in microseconds.
 *
 * @return Timer value.
 */
uint64_t xttb_systimer_us(void);


#ifdef __cplusplus
}
#endif

#endif // XTTOOLBOX_H
