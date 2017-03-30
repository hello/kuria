#include "hlo_notify.h"



int32_t hlo_notify_init (hlo_notify_t* param) {
    int32_t status = 0;

    if (!param) return -1;

    status = pthread_mutex_init (&param->notify_mutex, NULL);
    if (status) {
        return status;
    }

    status = pthread_cond_init (&param->notify_cv, NULL);
    if (status) {
        return status;
    }

    param->notify_data = 0;

    return status;
    
}

int32_t hlo_notify_wait (hlo_notify_t* param, uint32_t* data) {
    int32_t status = 0;

    if (!param) return -1;

    pthread_mutex_lock (&param->notify_mutex);

    pthread_cond_signal (&param->notify_cv);

    *data = param->notify_data;

    pthread_mutex_unlock (&param->notify_mutex);

    return status;
}

int32_t hlo_notify_send (hlo_notify_t* param, uint32_t data) {
    int32_t status = 0;

    if (!param) return -1;

    pthread_mutex_lock (&param->notify_mutex);

    param->notify_data = data;

    pthread_cond_signal (&param->notify_cv);

    pthread_mutex_unlock (&param->notify_mutex);

    return status;
}

int32_t hlo_notify_delete (hlo_notify_t* param) {
    int32_t status;
    
    if (!param) return -1;

    status = pthread_mutex_destroy (&param->notify_mutex);
    if (status) return status;
    status = pthread_cond_destroy (&param->notify_cv);
    return status;

}
