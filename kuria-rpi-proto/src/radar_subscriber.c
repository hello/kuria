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

#if (WRITE_PB_TO_FILE == 1)
#include "base64.h"
#endif

void* radar_subscriber (void) {

    radar_frame_packet_t packet;
    uint32_t data_index;
    int32_t status;

    printf("Starting file task\n");

    while(1) {

        // buffer for receiving the protobuf data through ipc
        uint8_t pb_buf[4096];

        // receive protobuf data
        int size = zmq_recv (subscriber, pb_buf, 4096, 0);

        // either write protobuf data, encoded as base64
        // or
        // write just baseband data to file

#if (WRITE_PB_TO_FILE == 1)
        // get the len for base64 encoding
        int encoded_len = Base64encode_len(size);

        // create buf to store base64 encoded data
        uint8_t* base64_buf = malloc (sizeof(uint8_t) * encoded_len);
        assert (base64_buf);

        // encode pb data as base64
        int len = Base64encode (base64_buf, pb_buf, size);

        if (len == encoded_len) {
            printf ("encoded length same\n");
        }
        else {
            printf ("len doesn't match\n");
        }

        // save to file 
        for (data_index = 0; data_index < len; data_index++) {
            fprintf (fp, "%u", base64_buf[data_index);
        }
        fprintf (fp, "\r\n");

        free (base64_buf);

#else
        //        printf ("size:%d\n",size);
        status = radar_data_decode (pb_buf, size, &packet);
        //        printf ("radar data decoded with :%d\n", status);

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

        free(packet.fdata);
#endif    

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
