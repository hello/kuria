/**
 * @file
 *
 * @brief System monitor task
 *
 * Monitors system health, e.g. tasks alive/messages. Resets watchdog if all seems OK.
 */

#ifndef TASK_MONITOR_H
#define TASK_MONITOR_H

#include "xep_dispatch.h"

typedef struct monitor_task_t_ {
    int id;
    char * name;
    volatile int tick_counter;
    int timeout_ms;
} monitor_task_t;

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize Monitor task.
 *
 * @return Status of execution
 */
uint32_t task_monitor_init(XepDispatch_t* dispatch);

/**
 * @brief Resgister a task to the monitor system.
 *
 * @param  monitor_task_handle Monitor task handle with info about the task registered.
 * @param  timeout_ms          Maximum time between calls to @ref monitor_task_alive
 * @return Status of execution as defined in @ref xt_error_codes_t
 */
int monitor_task_register(monitor_task_t ** monitor_task_handle, const uint32_t timeout_ms);

/**
 * @brief Signal "alive" to the monitor task
 *
 * @param  monitor_task_handle Initated monitor_task_handle - ref. @ref monitor_task_register
 * @return Status of execution as defined in @ref xt_error_codes_t
 */
uint32_t monitor_task_alive(monitor_task_t * monitor_task_handle);


#ifdef __cplusplus
}
#endif

#endif // TASK_MONITOR_H
