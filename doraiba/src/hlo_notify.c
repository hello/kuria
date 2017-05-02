#include "hlo_notify.h"
#include <stdio.h> 


int32_t hlo_notify_init (hlo_notify_t* param) {
    int32_t status = 0;

    if (!param) {
        printf ("hlo_notify_init: invalid param\n");
        return -1;
    }

    status = pthread_mutex_init (&param->notify_mutex, NULL);
    if (status) {
        printf ("hlo_notify_init: mutex init fail\n");
        return status;
    }

    status = pthread_cond_init (&param->notify_cv, NULL);
    if (status) {
        printf ("hlo_notify_init: cond variable init fail\n");
        return status;
    }

    param->notify_data = 0;

    return status;

}

int32_t hlo_notify_wait (hlo_notify_t* param, uint32_t* data, uint32_t bits) {
    int32_t status = 0;

    if (!param) { 
        printf ("hlo_notify_wait: invalid param\n");
        return -1;
    }

    pthread_mutex_lock (&param->notify_mutex);

    while ( (param->notify_data & bits) == 0) {
        pthread_cond_wait (&param->notify_cv, &param->notify_mutex);
    }

    *data = param->notify_data;
    param->notify_data = 0;

    pthread_mutex_unlock (&param->notify_mutex);

    return status;
}

int32_t hlo_notify_send (hlo_notify_t* param, uint32_t data) {
    int32_t status = 0;

    if (!param) { 
        printf ("hlo_notify_send: invalid param\n");
        return -1;
    }

    pthread_mutex_lock (&param->notify_mutex);

    param->notify_data = data;

    pthread_cond_signal (&param->notify_cv);

    pthread_mutex_unlock (&param->notify_mutex);

    return status;
}

int32_t hlo_notify_delete (hlo_notify_t* param) {
    int32_t status;

    if (!param) { 
        printf ("hlo_notify_delete: invalid param\n");
        return -1;
    }

    status = pthread_mutex_destroy (&param->notify_mutex);
    if (status){
        printf ("hlo_notify_delete: mutex destroy fail\n");
        return status;
    }
    status = pthread_cond_destroy (&param->notify_cv);
    if (status){
        printf ("hlo_notify_delete: cond variable destroy fail\n");
    }
    return status;

}
