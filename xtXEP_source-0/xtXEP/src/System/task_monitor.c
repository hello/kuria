/**
 * @file
 *
 *
 */

#include "task_monitor.h"
#include <FreeRTOS.h>
#include <task.h>
#include <queue.h>
#include "xtmemory.h"
#include "xep_hal.h"

#define TASK_MONITOR_STACK_SIZE            (500)
#define TASK_MONITOR_PRIORITY        (tskIDLE_PRIORITY + 0)

#define MAX_TASKS 10
#define MONITOR_TASK_CYCLE_TIME 100UL // 100ms

monitor_task_t * monitor_task_table[MAX_TASKS];

static void task_monitor(void *pvParameters);

/**
* Register task to monitor
*/
int monitor_task_register(monitor_task_t ** monitor_task_handle, const uint32_t timeout_ms)
{
    uint32_t status = XT_ERROR;

    // Find next empty table entry
    for (int i = 0; i < MAX_TASKS; i++)
    {
        if (monitor_task_table[i] == NULL)
        {
            // Create task table entry
            *monitor_task_handle = (monitor_task_t *)xtmemory_malloc_default(sizeof(monitor_task_t));
            (*monitor_task_handle)->id = i;
			(*monitor_task_handle)->name = pcTaskGetName(xTaskGetCurrentTaskHandle());
            (*monitor_task_handle)->timeout_ms = timeout_ms;
            (*monitor_task_handle)->tick_counter = (*monitor_task_handle)->timeout_ms / portTICK_PERIOD_MS / MONITOR_TASK_CYCLE_TIME;
            monitor_task_table[i] = *monitor_task_handle;
            status = XT_SUCCESS;
            break;
        }
    }
    return status;
}

/**
* Reset counter to indicate that task is alive
*/
uint32_t monitor_task_alive(monitor_task_t * monitor_task_handle)
{
    uint32_t status = XT_SUCCESS;
    if ((monitor_task_handle->id < MAX_TASKS) && (monitor_task_handle->id >= 0))
    {
        monitor_task_table[monitor_task_handle->id]->tick_counter = monitor_task_handle->timeout_ms / portTICK_PERIOD_MS / MONITOR_TASK_CYCLE_TIME;
    }
    else
    {
        status = XT_ERROR;
    }
    return status;
}

uint32_t task_monitor_init(XepDispatch_t* dispatch)
{
    TaskHandle_t h_task_monitor;

    for (int i = 0; i < MAX_TASKS; i++)
    {
        monitor_task_table[i] = NULL;
    }

    xTaskCreate(task_monitor, (const char * const) "System", TASK_MONITOR_STACK_SIZE, (void*)dispatch, TASK_MONITOR_PRIORITY, &h_task_monitor);

    return 0;
}

static void task_monitor(void *pvParameters)
{
    XepDispatch_t* dispatch = (XepDispatch_t*)pvParameters;
    UNUSED(dispatch);

    TickType_t last_wake_time = xTaskGetTickCount();

    for (;;)
    {
        // Update tick counter for each task
        for (int i = 0; i < MAX_TASKS; i++)
        {
            if (monitor_task_table[i] != NULL)
            {
                if (monitor_task_table[i]->tick_counter < 0)
                {
                    // task timed out
				    xtio_led_set_state(XTIO_LED_RED, XTIO_LED_ON);
				    xtio_led_set_state(XTIO_LED_GREEN, XTIO_LED_OFF);
				    xtio_led_set_state(XTIO_LED_BLUE, XTIO_LED_OFF);

                    // TODO: Try to send message to host??

					// Store reset reason. and reset
					xt_software_reset(XT_SWRST_SYSTEM_MONITOR);
					while(1)
					{
						// If reset works, we should not come here
						int dummy = 1;
                        (void)dummy;
					}
                }
                monitor_task_table[i]->tick_counter--;
            }
        }
        xt_feed_watchdog();
        vTaskDelayUntil(&last_wake_time, MONITOR_TASK_CYCLE_TIME / portTICK_PERIOD_MS);
    }
}
