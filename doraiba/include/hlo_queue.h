#ifndef __HLO_QUEUE_H__
#define __HLO_QUEUE_H__

#include <stdint.h>
#include <stdbool.h>
#include <pthread.h>
#include "radar_data_format.h"

#define HLO_QUEUE_ID_INVALID -1
#define HLO_QUEUE_MAX 50


typedef struct {
    // mutex 
    pthread_mutex_t queue_mutex;

    // cond variable for blocking
    pthread_cond_t queue_cv;

    // pointer to hold data
    radar_frame_packet_t** data;

    // number of items
    uint32_t queue_size; 

    // number of items currently in queue
    uint32_t number_of_items;

    // Read index
    uint32_t read_index;

    // Write index
    uint32_t write_index;
} hlo_queue_t;

int32_t hlo_queue_create (hlo_queue_t* queue, uint32_t num_of_items);
int32_t hlo_queue_delete (hlo_queue_t* queue);
int32_t hlo_queue_send (hlo_queue_t* queue, radar_frame_packet_t* data, uint32_t timeout);
int32_t hlo_queue_recv (hlo_queue_t* queue, radar_frame_packet_t* data, uint32_t timeout);
bool hlo_queue_is_full (hlo_queue_t* queue);
bool hlo_queue_is_empty (hlo_queue_t* queue);

#endif
