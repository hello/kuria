#include "debug_publisher.h"
#include <cstdlib>

DebugPublisherInterface * DebugPublisher::_instance = nullptr;

class DefaultDebugPublisher : public DebugPublisherInterface {
    void publish(const std::string & id, const Eigen::MatrixXcf & mat) {
        
    }
    
    void publish(const std::string & id, const Eigen::MatrixXf & mat) {
        
    }
};


DebugPublisherInterface * DebugPublisher::get_instance() {
    
    if (!_instance) {
        _instance = new DefaultDebugPublisher();
    }
    
    return _instance;
}

void DebugPublisher::initialize(DebugPublisherInterface *publisher) {
    if (_instance) {
        deinitialize();
    }
    
    _instance = publisher;
}

void DebugPublisher::deinitialize() {
    if (_instance) {
        delete _instance;
        _instance = nullptr;
    }
}








