#include <zmq.h>
#include <assert.h>
#include <unistd.h>
#include "zmq_endpoint.h"
#include "string.h"

int main (void) {

    // socket to talk to clients
    void* context = zmq_ctx_new();
    void* publisher = zmq_socket (context,ZMQ_PUB );
#if USE_IPC
    int rc = zmq_bind (publisher,ZMQ_ENDPOINT); 
    printf ("using ipc\n");
#else
    int rc = zmq_bind (publisher, "tcp://*:5556");
#endif
    assert (rc == 0);


    while (1) {

        printf ("sending Hello\n");
        zmq_send (publisher, "Hello", strlen ("Hello"), 0);
        sleep (5);
    }

    zmq_close (publisher);
    zmq_ctx_destroy (context);
    
    return 0;
}
