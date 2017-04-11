#include <stdio.h>
#include <fcntl.h>
#include <stdint.h>
#include "kuria_config.h"
#include <math.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <zmq.h>
#include <assert.h>
#include <unistd.h>
#include "radar_data_format.h"
#include "novelda_protobuf.h"

// file for radar data
FILE* fp;

// zmq subscriber
void* context;
void* subscriber;

int32_t radar_subscriber_init (void);
void* radar_subscriber (void); 
int32_t file_close(void);

int32_t radar_subscriber_init (void) {

    int status;
    char filename[40];
    time_t now = time(NULL);

    // set filename
    sprintf(filename, "data_%d", (int)now);

    // add file extension
    strcat(filename, ".csv");

    printf("Opening file: %s\n", filename);

    // open file
    fp = fopen(filename,"w+");
    if(fp == NULL ){
        printf("cannot open file \n");
        return -1;
    }

    printf(" File init done\n");


    // setup ZMQ subscriber to subscribe to radar data
    context = zmq_ctx_new();
    subscriber = zmq_socket (context,ZMQ_SUB );
    int rc = zmq_connect (subscriber, ZMQ_ENDPOINT);

    assert (rc == 0);

    // TODO how to set this for radar data
    zmq_setsockopt (subscriber, ZMQ_SUBSCRIBE, "", 0);

    return 0;
}

void* radar_subscriber (void) {

    radar_frame_packet_t packet;
    uint32_t data_index;
    int32_t status;

    printf("Starting file task\n");

    while(1) {

        uint8_t pb_buf[4096];
        novelda_RadarFrame frame;
#if 1
        int size = zmq_recv (subscriber, pb_buf, 4096, 0);
        printf ("Received pb of size:%d\n",size);
#else
        char buffer[10] = {0};
        int size = zmq_recv (subscriber, buffer, 10, 0);
        printf ("Received: %s of size:%d\n", buffer, size);
#endif
#if 0
        status = radar_data_decode ((uint8_t*) &frame, size, &packet);
        printf ("radar data decoded with :%d\n", status);
#endif
        /*
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
        */

    }

    file_close ();

    zmq_close (subscriber);
    zmq_ctx_destroy (context);

}

int32_t file_close(void){
    int32_t status = 0;
    if(fp){
        status = fclose(fp);
    }
    return status;

}

int main (void) {

    // initialize radar subcriber
    radar_subscriber_init ();

    // start radar subscriber
    radar_subscriber ();

    return 0;
}
