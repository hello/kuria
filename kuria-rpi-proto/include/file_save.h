#ifndef _FILE_SAVE_H_
#define _FILE_SAVE_H_



#include <stdint.h>
#include "kuria_config.h"

#if USE_FREERTOS_TASKS
int32_t file_task_init(void); 
void file_task(void* pvParameters); 
#else
int32_t file_task_init (pthread_t* thread_id);
void* file_task (void* param); 
#endif

int32_t file_close(void);



#endif
