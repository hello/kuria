#include <stdio.h>
#include <zmq.h>
#include <assert.h>
#include "dispatch_radar_frame.h"
#include <stdint.h>
#include "kuria_config.h"
#include "novelda_protobuf.h"
#include <stdlib.h>

void* context;
void* publisher;


int32_t dispatcher_init (void) {

    // setup zmq to publish radar data
    context = zmq_ctx_new();
    publisher = zmq_socket (context,ZMQ_PUB );
    int rc = zmq_bind (publisher,ZMQ_ENDPOINT); 
    assert (rc == 0);
    return 0;
}

int32_t dispatch_radar_frame (radar_frame_packet_t* packet) {
    int32_t status;
    uint8_t* pb_buf;

    // encode protbuf
    status = radar_data_encode (&pb_buf, packet);
    if (status == -1) {
        printf ("radar data encode fail\n");
        return status;
    }

    size_t len = (size_t) status;

    // publish radar data
    do {
    status = zmq_send (publisher, pb_buf , len, 0);
    } while ( (status == -1) && ( (errno == EAGAIN) || (errno == EINTR) ) );
    free (pb_buf);
    if (status == -1) {
        perror ("zmq send fail ");
        return status;
    }
    else {
//        printf ("zmq sent: %d\n", status);
    }
    
    return 0;
}

int32_t dispatcher_close (void) {

    zmq_close (publisher);
    zmq_ctx_destroy (context);

    return 0;
}
