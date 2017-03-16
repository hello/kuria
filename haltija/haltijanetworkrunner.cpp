
#include "network/zmq_subscriber.h"
class TestMessage {

};

class Subscriber {
public:
    void receive_message(const TestMessage & message) {
        
    }
};


class Deserializer {
public:
    bool deserialize_protobuf(const uint8_t * bytes, const size_t num_bytes, TestMessage & testmessage) {
        
        std::cout << "got message of " << num_bytes << "len" << std::endl;
    
        return true;
    }
};

int main() {
    
    ZmqSubscriber<Deserializer,TestMessage,Subscriber> zmq_subscriber(10000,"tcp://localhost:5563");
    
    zmq_subscriber.add_subscriber("foobars", new Subscriber());
    
    zmq_subscriber.run();
    

    return 0;
}
