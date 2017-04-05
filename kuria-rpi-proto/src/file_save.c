#include "file_save.h"
#include <stdio.h>
#include <fcntl.h>
#include <math.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>


#include <unistd.h>
#include "radar_data_format.h"
#include "hlo_queue.h"

FILE* fp;

hlo_queue_t radar_data_queue;

extern int32_t radar_data_frame_free( radar_frame_packet_t* packet ); 

int32_t file_task_init (pthread_t* thread_id) {

    int status;
    pthread_t file_task_thread_id;
    char filename[40];
    time_t now = time(NULL);
    sprintf(filename, "data_%d", (int)now);
    strcat(filename, ".csv");
    printf("Opening file: %s\n", filename);
    fp = fopen(filename,"w+");
    if(fp == NULL ){
        printf("cannot open file \n");
        return -1;
    }



    // Create queue for data transfer with radar task
    status = hlo_queue_create (&radar_data_queue, 25); // TODO remove magic number
    if (status) {
        printf ("error creating queue\n");
        return status;
    }

    pthread_attr_t file_task_attr;

    pthread_attr_init (&file_task_attr);
    pthread_attr_setdetachstate (&file_task_attr, PTHREAD_CREATE_DETACHED);

    printf ("creating file task thread\n");


    status = pthread_create (&file_task_thread_id, &file_task_attr, file_task, NULL);
    if (status) {
        printf ("error creating file task thread: %d\n", status);
        return status;
    }
    // TODO redundant variable can be removed 
    *thread_id = file_task_thread_id;

    printf(" File init done\n");
    return 0;
}

void* file_task (void* param) {

    radar_frame_packet_t packet;

    uint32_t data_index;

    printf("Starting file task\n");

    while(1) {
        // receive data from queue
        //
        //printf("wait for data\n");
        if (hlo_queue_recv (&radar_data_queue, &packet, 0) == 0) {

            if( !packet.fdata ) {
                printf(" invalid data \n" );
                continue;
            }

            fprintf(fp, "\r\n");

            //printf("Data received\n");
            for(data_index = 0; data_index <= packet.num_of_bins-2; data_index+=2) {
                // save to file
                //
                fprintf( fp, "%f%c%fi",  packet.fdata[data_index],( (packet.fdata[data_index+1] >= 0) ? '+':'-') , fabs (packet.fdata[data_index+1]) );
                if (data_index != packet.num_of_bins-2) {
                    fprintf (fp, ",");
                }
            }

            printf ("wrote\n");
            free (packet.fdata);
            // free pointers to radar frame data
            // 
            //            radar_data_frame_free( packet );
            //printf("wr done\n");
        }

    }

}

int32_t file_close(void){
#if USE_FREERTOS_TASKS
    if( radar_data_queue) vQueueDelete( radar_data_queue );
#endif
    int32_t status = 0;
    if(fp){
        status = fclose(fp);
    }
    return status;

}
