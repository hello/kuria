#include "file_save.h"
#include <stdio.h>
#include <fcntl.h>
#include <math.h>
#include <time.h>
#include <string.h>

#if USE_FREERTOS_TASKS
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#endif

#include <unistd.h>
#include "radar_data_format.h"

#if USE_FREERTOS_TASKS
#define FILE_TASK_STACK_SIZE            (1500)
#define FILE_TASK_PRIORITY        (tskIDLE_PRIORITY + 6)
#endif

FILE* fp;

#if USE_FREERTOS_TASKS
static TaskHandle_t h_task_file = NULL;
QueueHandle_t radar_data_queue; 
#endif

extern int32_t radar_data_frame_free( radar_frame_packet* packet ); 

#if USE_FREERTOS_TASKS
int32_t file_task_init(void) {
#else
int32_t file_task_init (pthread_t* thread_id) {
#endif

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
#if USE_FREERTOS_TASKS
    xTaskCreate(file_task, (const char* const) "file_save", FILE_TASK_STACK_SIZE, \
           NULL , FILE_TASK_PRIORITY, &h_task_file);

    radar_data_queue = xQueueCreate(50 , sizeof(radar_frame_packet* ) );

    if( radar_data_queue == NULL ) {
        printf("error creating radar data queue\n");
        file_close();
        return -1;
    }
#else

    pthread_attr_t file_task_attr;

    pthread_attr_init (&file_task_attr);
    pthread_attr_setdetachstate (&file_task_attr, PTHREAD_CREATE_DETACHED);

    printf ("creating file task thread\n");

    pthread_t file_task_thread_id;
    int status;

    status = pthread_create (&file_task_thread_id, &file_task_attr, file_task, NULL);
    if (status) {
        printf ("error creating file task thread: %d\n", status);
        return status;
    }
    
    // TODO redundant variable can be removed 
    *thread_id = file_task_thread_id;

#endif
    printf(" File init done\n");
    return 0;
}

#if USE_FREERTOS_TASKS
void file_task(void* pvParameters) {
#else
void* file_task (void* param) {
#endif

    radar_frame_packet* packet = NULL;
    uint32_t data_index;

    printf("Starting file task\n");

    while(1) {
        // receive data from queue
        //
        //printf("wait for data\n");
        packet = NULL;
#if USE_FREERTOS_TASKS
        if( xQueueReceive( radar_data_queue, &packet, portMAX_DELAY ) ) 
#else
            printf ("pausing file task thread\n");
            pause ();
#endif 
        {
            if( !packet->fdata ) {
                printf(" invalid data \n" );
                continue;
            }
           
            fprintf(fp, "\r\n");

            //printf("Data received\n");
            for(data_index = 0; data_index <= packet->num_of_bins-2; data_index+=2) {
                // save to file
                //
                fprintf( fp, "%f%c%fi",  packet->fdata[data_index],( (packet->fdata[data_index+1] >= 0) ? '+':'-') , fabs (packet->fdata[data_index+1]) );
                if (data_index != packet->num_of_bins) {
                    fprintf (fp, ",");
                }
            }
            
            // free pointers to radar frame data
            // 
            radar_data_frame_free( packet );
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
