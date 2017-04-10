#include <stdio.h>
#include <fcntl.h>
#include <pthread.h>
#include <stdint.h>
#include "kuria_config.h"
#include <math.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>


#include <unistd.h>
#include "radar_data_format.h"

FILE* fp;


int32_t file_task_init (pthread_t* thread_id);
void* file_task (void* param); 
int32_t file_close(void);

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
    int32_t status;

    printf("Starting file task\n");

    while(1) {
        //
        //printf("wait for data\n");
        /*
        if ( (status=hlo_queue_recv (&radar_data_queue, &packet, 0) ) == 0) {

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

            printf (".");
            if( packet.fdata ) {
                free(packet.fdata);
            }
        }
        else {
            printf ("hlo_queue_recv err: %d\n", status);
        }

        */
    }

}

int32_t file_close(void){
    int32_t status = 0;
    if(fp){
        status = fclose(fp);
    }
    return status;

}

int main (void) {



    return 0;
}
