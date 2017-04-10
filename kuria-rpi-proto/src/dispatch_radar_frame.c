#include <stdio.h>
#include <zmq.h>
#include <assert.h>
#include "dispatch_radar_frame.h"
#include <stdint.h>
#include "kuria_config.h"
#include "novelda_protobuf.h"


void* context;
void* publisher;

// TODO verify all ZMQ return values are handled accordingly

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
    novelda_RadarFrame frame;

    // encode protbuf
    status = radar_data_encode (&frame, packet);
    if (status) {
        printf ("radar data encode fail\n");
        return status;
    }

    // publish radar data
    status = zmq_send (publisher, "Hello", strlen ("Hello"), 0);
    if (status) {
        printf ("zmq send fail\n");
        return status;
    }
    
    return 0;
}

int32_t dispatcher_close (void) {

    zmq_close (publisher);
    zmq_ctx_destroy (context);

    return 0;
}
