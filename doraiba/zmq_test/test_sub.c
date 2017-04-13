#include <zmq.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>
#include "zmq_endpoint.h"

int main (void) {

    // socket to talk to clients
    void* context = zmq_ctx_new();
    void* subscriber = zmq_socket (context,ZMQ_SUB );
    int rc = zmq_connect (subscriber, ZMQ_ENDPOINT);

    assert (rc == 0);

    zmq_setsockopt (subscriber, ZMQ_SUBSCRIBE, "Hello", strlen ("Hello"));

    while (1) {

        char buffer[10] = {0};
        int size = zmq_recv (subscriber, buffer, 10, 0);
        printf ("Received: %s of size:%d\n", buffer, size);
    }

    zmq_close (subscriber);
    zmq_ctx_destroy (context);
    return 0;
}
