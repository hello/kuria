#ifndef __RADAR_TASK_H__
#define __RADAR_TASK_H__

#include "kuria_config.h"

int32_t radar_task_init (pthread_t* thread_id); 
void* radar_task(void* params);
void radar_task_end (void); 


#endif
