#include <zmq.h>
#include <assert.h>
#include <unistd.h>

char endpoint[] = "ipc://mylocal/";

int main (void) {

    // socket to talk to clients
    void* context = zmq_ctx_new();
    void* publisher = zmq_socket (context,ZMQ_PUB );
    int rc = zmq_bind (publisher, endpoint);
    assert (rc == 0);


    while (1) {

        printf ("sending Hello\n");
        zmq_send (publisher, "Hello", 5, 0);
        sleep (5);
    }

    zmq_close (publisher);
    zmq_ctx_destroy (context);
    
    return 0;
}
