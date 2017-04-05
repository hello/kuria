#include "hlo_queue.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int32_t hlo_queue_create (hlo_queue_t* queue, uint32_t num_of_items) {

    if (!queue) {
        printf ("invalid queue\n");
    }

    queue->number_of_items = 0;
    queue->queue_size = num_of_items;

    queue->write_index = 0;
    queue->read_index = 0;

    int32_t status = 0;

    status = pthread_mutex_init (&queue->queue_mutex, NULL);
    if (status) {
        return status;
    }

    status = pthread_cond_init (&queue->queue_cv, NULL);
    if (status) {
        return status;
    }

    // malloc memory for array for items
    queue->data = (radar_frame_packet_t**) malloc (num_of_items * sizeof (radar_frame_packet_t) );
    if ( !(queue->data) ) {
        printf ("error creating queue - memory\n");
        return -1;
    }

    return 0;
}

int32_t hlo_queue_delete (hlo_queue_t* queue){

    int32_t status;
    
    if (!queue) return -1;

    status = pthread_mutex_destroy (&queue->queue_mutex);
    if (status) return status;
    status = pthread_cond_destroy (&queue->queue_cv);
    if (status) return status;

    free (queue->data);

    return 0;

}

int32_t hlo_queue_send (hlo_queue_t* queue, radar_frame_packet_t* data, uint32_t timeout) {

    if (!queue || !data ) return -1;

    pthread_mutex_lock (&(queue->queue_mutex));

    if (queue->queue_size == queue->number_of_items ) return -2;

    memcpy ( &queue->data[queue->write_index], data, sizeof (radar_frame_packet_t) );

//    queue->write_index = (queue->write_index + 1) % queue->queue_size;

    queue->number_of_items++;

    pthread_cond_signal (&queue->queue_cv);

    pthread_mutex_unlock (&queue->queue_mutex);

    return 0;

}

int32_t hlo_queue_recv (hlo_queue_t* queue, radar_frame_packet_t* data, uint32_t timeout) {

    if (!queue || !data ) return -1;

    pthread_mutex_lock (&queue->queue_mutex);

    while (0 == queue->number_of_items ) {
        pthread_cond_wait (&queue->queue_cv, &queue->queue_mutex);
    }

    memcpy (data, &queue->data[queue->read_index], sizeof (radar_frame_packet_t) );

//    queue->read_index = (queue->read_index + 1) % queue->queue_size;

    queue->number_of_items--;

    pthread_mutex_unlock (&queue->queue_mutex);

    return 0;
}

bool hlo_queue_is_full (hlo_queue_t* queue) {

    if (!queue) return true;

    pthread_mutex_lock (&queue->queue_mutex);

    if (queue->queue_size == queue->number_of_items) return true;


    pthread_mutex_unlock (&queue->queue_mutex);

    return false;
}

bool hlo_queue_is_empty (hlo_queue_t* queue) {

    if (!queue) return false;


    pthread_mutex_lock (&queue->queue_mutex);

    if (0 == queue->number_of_items) return true;
     
    pthread_mutex_unlock (&queue->queue_mutex);

    return false;
}
