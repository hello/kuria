
#include "network/zmq_subscriber.h"
#include "network/zmq_publisher.h"

#include "network/noveldaprotobuf.h"
#include "network/radarmessageprotobuf.h"
#include "algorithm/novelda_radar_subscriber.h"
#include "radar_result_publisher_interface.h"
#include "haltija_types.h"



class RadarMessagePublisher : public RadarResultPublisherInterface {
public:
    RadarMessagePublisher() {
        _publisher.initialize("tcp://*:5564");
    }
    
    void publish(const char * prefix, const RadarMessage_t & message) {
        _publisher.publish(prefix, message);
    }
    
private:
    
    ZmqPublisher<RadarMessageProtobuf,RadarMessage_t> _publisher;
};

int main(int argc, char * argv[]) {
    
    ZmqSubscriber<NoveldaProtobuf,NoveldaData_t,NoveldaRadarSubscriber> zmq_subscriber(100000,"tcp://localhost:5563");
    
    zmq_subscriber.add_subscriber("foobars", new NoveldaRadarSubscriber(new RadarMessagePublisher()));
    
    zmq_subscriber.run();
    
    return 0;
}
