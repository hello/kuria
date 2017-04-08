#include <zmq.h>
#include <assert.h>
#include <unistd.h>

char endpoint[] = "ipc://mylocal/";

int main (void) {

    // socket to talk to clients
    void* context = zmq_ctx_new();
    void* subscriber = zmq_socket (context,ZMQ_SUB );
    int rc = zmq_connect (subscriber, endpoint);
    assert (rc == 0);


    while (1) {

        char buffer[10];
        zmq_recv (subscriber, buffer, 10, 0);
        printf ("Received: %s\n", buffer);
        sleep (5);
    }

    zmq_close (subscriber);
    zmq_ctx_destroy (context);
    return 0;
}
