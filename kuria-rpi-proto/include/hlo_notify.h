#ifndef __HLO_NOTIFY_H__
#define __HLO_NOTIFY_H__

#include <stdint.h>
#include <pthread.h>

typedef struct {
    // mutex 
    pthread_mutex_t notify_mutex;
    // cond variable
    pthread_cond_t notify_cv;
    // flag
    uint32_t notify_data;
}hlo_notify_t;

int32_t hlo_notify_init (hlo_notify_t* param);
int32_t hlo_notify_wait (hlo_notify_t* param, uint32_t* data);
int32_t hlo_notify_send (hlo_notify_t* param, uint32_t data);
int32_t hlo_notify_delete (hlo_notify_t* param);

#endif
