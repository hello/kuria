#include <stdio.h>
#include <zmq.h>
#include <assert.h>
#include "dispatch_radar_frame.h"
#include <stdint.h>
#include "kuria_config.h"


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

    // encode protbuf
    //
    // publish radar data
    //

    return 0;
}
