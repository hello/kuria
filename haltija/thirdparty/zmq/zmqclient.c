#include <zmq.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>

static void s_recv (void *socket, char * buffer) {
    int size = zmq_recv (socket, buffer, 255, 0);
    buffer[size] = '\0';
}

static int b_list(void * socket) {
   char buffer [256];
   int size = zmq_recv (socket, buffer, 255, 0);
   return size;
}


int main (void)
{
    //  Prepare our context and subscriber
    void *context = zmq_ctx_new ();
    void *subscriber = zmq_socket (context, ZMQ_SUB);
    zmq_connect (subscriber, "tcp://127.0.0.1:5564");
    zmq_setsockopt (subscriber, ZMQ_SUBSCRIBE, NULL, 0);

    while (1) {
        //  Read envelope with address
        char buffer[256] = {0};
        s_recv (subscriber,buffer);
        //  Read message contents
        int nbytes = b_list(subscriber);
        printf ("[%s] %d\n", buffer, nbytes);
    }
    //  We never get here, but clean up anyhow
    zmq_close (subscriber);
    zmq_ctx_destroy (context);
    return 0;
}
