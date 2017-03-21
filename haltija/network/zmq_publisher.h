#ifndef _ZMQPUBLISHER_H_
#define _ZMQPUBLIHSER_H_

#include "zhelpers.h"
#include <unistd.h>
#include <cstdlib>

template <class Serializer,class Message>
class ZmqPublisher {
public:
    ZmqPublisher() : _context(NULL),_publisher(NULL) {}

    ~ZmqPublisher() {
        if (_publisher) {
            zmq_close(_publisher);
        }
 
        if (_context) {
            zmq_ctx_destroy(_context);
        }
    }

    void initialize(const char * target) {
        _context = zmq_ctx_new();
        _publisher = zmq_socket (_context, ZMQ_PUB);
    }

    bool publish(const char * prefix, const Message & message) {
        if (!_context || !_publisher) {
            return false;
        }

        if (prefix) {
            s_sendmore (_publisher, prefix); 
        }

        uint8_t * null_terminated_bytes = Serializer::serialize_protobuf(message);

        if (null_terminated_bytes) {
            s_send (_publisher,null_terminated_bytes);   
            free(null_terminated_bytes);
        }
    }

private:
    void * _context;
    void * _publisher;
};


#endif //_ZMQPUBLISHER_H_
