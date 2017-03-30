#ifndef _FILE_SAVE_H_
#define _FILE_SAVE_H_



#include <stdint.h>
#include "kuria_config.h"
#include <pthread.h>

int32_t file_task_init (pthread_t* thread_id);
void* file_task (void* param); 
int32_t file_close(void);



#endif
