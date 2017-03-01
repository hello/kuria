/**
 * @file
 *
 * @brief Radar event task
 *
 * Serves radar events, such as data ready, reads data and sends it to subscriber.
 */

#ifndef TASK_RADAR_H
#define TASK_RADAR_H

#include "xep_dispatch.h"
#include "x4driver.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize Radar task.
 *
 * @return Status of execution
 */
uint32_t task_radar_init(X4Driver_t** x4driver, XepDispatch_t* dispatch);


#ifdef __cplusplus
}
#endif

#endif // TASK_RADAR_H
