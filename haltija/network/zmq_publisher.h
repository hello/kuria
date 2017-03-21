#ifndef _ZMQPUBLISHER_H_
#define _ZMQPUBLISHER_H_

#include <unistd.h>
#include <cstring>

#define PREFIX_BUF_SIZE (1024)

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
        char prefix_buf[PREFIX_BUF_SIZE] = {0};
        
        strncpy(prefix_buf,prefix,PREFIX_BUF_SIZE);
        
        if (!_context || !_publisher) {
            return false;
        }

        if (prefix) {
            int sent_size = s_sendmore (_publisher, (const uint8_t *)prefix,strnlen(prefix,PREFIX_BUF_SIZE));
            
            std::cout << "sent " << sent_size << " prefix bytes, ";
        }
        
        size_t message_size = 0;
        uint8_t * null_terminated_bytes = Serializer::serialize_protobuf(message,message_size);

        if (null_terminated_bytes) {
            int sent_size = s_send (_publisher,null_terminated_bytes,message_size);
            
            std::cout << "sent " << sent_size << " payload bytes" <<std::endl;

            
            free(null_terminated_bytes);
        }
        
        return true;
    }

private:
    void * _context;
    void * _publisher;
    
    int s_sendmore (void *socket, const uint8_t * bytes, const size_t size) {
        int sent_size = zmq_send (socket, bytes, size, ZMQ_SNDMORE);
        return sent_size;
    }
    
    int s_send (void *socket,  const uint8_t * bytes,const size_t size) {
        int sent_size = zmq_send (socket, bytes, size, 0);
        return sent_size;
    }
};


#endif //_ZMQPUBLISHER_H_
