#include <stdio.h>
#include <fcntl.h>
#include <math.h>
#include <time.h>
#include <string.h>
#include "FreeRTOS.h"
#include "task.h"
#include <unistd.h>
#include "file_save.h"
#include "task.h"
#include "queue.h"
#include "radar_data_format.h"

#define FILE_TASK_STACK_SIZE            (1500)
#define FILE_TASK_PRIORITY        (tskIDLE_PRIORITY + 6)


FILE* fp;
static TaskHandle_t h_task_file = NULL;
QueueHandle_t radar_data_queue; 
extern int32_t radar_data_frame_free( radar_frame_packet* packet ); 

int32_t file_task_init(void) {

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
    xTaskCreate(file_task, (const char* const) "file_save", FILE_TASK_STACK_SIZE, \
           NULL , FILE_TASK_PRIORITY, &h_task_file);

    radar_data_queue = xQueueCreate(50 , sizeof(radar_frame_packet* ) );

    if( radar_data_queue == NULL ) {
        printf("error creating radar data queue\n");
        file_close();
        return -1;
    }
    printf(" File init done\n");
    return 0;
}


void file_task(void* pvParameters) {

    radar_frame_packet* packet = NULL;
    uint32_t data_index;

    printf("Starting file task\n");

    while(1) {
        // receive data from queue
        //
        //printf("wait for data\n");
        packet = NULL;
        if( xQueueReceive( radar_data_queue, &packet, portMAX_DELAY ) ) {
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

    vTaskDelete( NULL );
}

int32_t file_close(void){
    if( radar_data_queue) vQueueDelete( radar_data_queue );
    int32_t status = 0;
    if(fp){
        status = fclose(fp);
    }
    return status;

}
