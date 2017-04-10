
#include "network/zmq_subscriber.h"
#include "network/zmq_publisher.h"

#include "network/noveldaprotobuf.h"
#include "network/radarmessageprotobuf.h"
#include "algorithm/novelda_radar_subscriber.h"
#include "radar_result_publisher_interface.h"
#include "haltija_types.h"

const char * publish_host_port = "tcp://127.0.0.1:5564";
const char * subscribe_host_port = "tcp://127.0.0.1:5563";

class RadarMessagePublisher : public RadarResultPublisherInterface {
public:
    RadarMessagePublisher() {
        _publisher.initialize(publish_host_port);
    }
    
    void publish(const char * prefix, const RadarMessage_t & message) {
        _publisher.publish(prefix, message);
    }
    
private:
    
    ZmqPublisher<RadarMessageProtobuf,RadarMessage_t> _publisher;
};

int main(int argc, char * argv[]) {
    
    ZmqSubscriber<NoveldaProtobuf,NoveldaData_t,NoveldaRadarSubscriber> zmq_subscriber(100000,subscribe_host_port);
    
    zmq_subscriber.add_subscriber("foobars", new NoveldaRadarSubscriber(new RadarMessagePublisher(),NULL));
    
    zmq_subscriber.run();
    
    return 0;
}
