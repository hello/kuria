
#include "network/zmq_subscriber.h"
#include "network/zmq_publisher.h"

#include "network/noveldaprotobuf.h"
#include "network/radarmessageprotobuf.h"
#include "algorithm/novelda_radar_subscriber.h"
#include "radar_result_publisher_interface.h"
#include "haltija_types.h"
#include "yaml-cpp/yaml.h"



class RadarMessagePublisher : public RadarResultPublisherInterface {
public:
    RadarMessagePublisher(const std::string & publish_host_port) {
        _publisher.initialize(publish_host_port.c_str());
    }
    
    void publish(const char * prefix, const RadarMessage_t & message) {
        _publisher.publish(prefix, message);
    }
    
private:
    
    ZmqPublisher<RadarMessageProtobuf,RadarMessage_t> _publisher;
};

int main(int argc, char * argv[]) {
    
    
    if (argc < 2) {
        std::cerr << "requires config yml format config file" << std::endl;
        return 0;
    }
    
    
    YAML::Node config = YAML::LoadFile(argv[1]);
    
    if (!config["result_publish_address"]) {
        std::cerr << "result_publish_address field not found in " << argv[1] << std::endl;
        return 0;
    }
    
    if (!config["radar_subscribe_address"]) {
        std::cerr << "radar_subscribe_address field not found in " << argv[1] << std::endl;
        return 0;
    }
    
    if (!config["radar_subscribe_prefix"]) {
        std::cerr << "radar_subscribe_prefix field not found in " << argv[1] << std::endl;
        return 0;
    }
    

    std::string publish_host_port = config["result_publish_address"].as<std::string>();
    std::string subscribe_host_port = config["radar_subscribe_address"].as<std::string>();
    std::string radar_subscribe_prefix = config["radar_subscribe_prefix"].as<std::string>();

    std::cout << "publish to: " << publish_host_port << std::endl;
    std::cout << "subscribe from: " << subscribe_host_port << std::endl;
    std::cout << "subscribe channel: " << radar_subscribe_prefix << std::endl;

    
    ZmqSubscriber<NoveldaProtobuf,NoveldaData_t,NoveldaRadarSubscriber> zmq_subscriber(100000,subscribe_host_port.c_str());
    
    zmq_subscriber.add_subscriber(radar_subscribe_prefix.c_str(), new NoveldaRadarSubscriber(new RadarMessagePublisher(publish_host_port),NULL));
    
    zmq_subscriber.run();
    
    return 0;
}
