#include "novelda_radar_subscriber.h"
#include <iostream>
#include "respiration.h"
#include "log.h"
#include <unistd.h>

#define EXPECTED_SAMPLE_RATE_HZ (20)
#define NUM_FRAMES_IN_SEGMENT (5 * EXPECTED_SAMPLE_RATE_HZ)
#define NUM_FRAMES_TO_WAIT (5 * EXPECTED_SAMPLE_RATE_HZ)


using namespace Eigen;


NoveldaRadarSubscriber::NoveldaRadarSubscriber(RadarResultPublisherInterface * publisher)
:_publisher(publisher)
,_sequence_number(0LL) {
    _preprocessor = PreprocessorPtr_t(NULL);
    
    for (int i = 8; i < 38; i++) {
        _rangebins_we_care_about.insert(i);
    }
    
}

NoveldaRadarSubscriber::~NoveldaRadarSubscriber() {
    
}


void NoveldaRadarSubscriber::receive_message(const NoveldaData_t & message) {
    
    if (!_preprocessor.get()) {
        //TODO configure this from constructor
        _preprocessor = Preprocessor::createWithDefaultHighpassFilter(message.range_bins.size(), NUM_FRAMES_IN_SEGMENT, NUM_FRAMES_TO_WAIT);
    }
    
    std::cout << "frame " << message.frame_id << std::endl;

    BasebandDataFrame_t frame;
    frame.data.reserve(message.range_bins.size());
    
    for (auto it = message.range_bins.begin(); it != message.range_bins.end(); it++) {
        frame.data.push_back(Complex_t((*it).real(),(*it).imag()));
    }
       
    MatrixXcf filtered_frame;
    MatrixXcf segment;
    
    uint32_t flags = _preprocessor->add_frame(frame, filtered_frame, segment);
    
    //DO KALMAN FILTERS HERE
    if (~flags & PREPROCESSOR_FLAGS_FRAME_READY) {
        return;
    }
    
    if (flags & PREPROCESSOR_FLAGS_SEGMENT_READY) {
        //DO FRAME PROCESSING HERE
        _combiner.set_latest_segment(segment, _rangebins_we_care_about);
    }
    
    MatrixXcf transformed_frame;
    if (_combiner.get_latest_reduced_measurement(filtered_frame, transformed_frame)) {

        //send frame over the wire, or process it or something
        
        if (_publisher) {
#define HOST_NAME_MAX (1024)
#define LOGIN_NAME_MAX (1024)
            char hostname[HOST_NAME_MAX] = {0};
            char username[LOGIN_NAME_MAX] = {0};
            gethostname(hostname, HOST_NAME_MAX);
            getlogin_r(username, LOGIN_NAME_MAX);
            
            RadarMessage_t message;
            message.sequence_number = _sequence_number;
            message.vec.push_back(transformed_frame(0,0).real());
            message.vec.push_back(transformed_frame(0,0).imag());
            message.id = username;
            message.device_id = hostname;
            
            std::cout << "publish:" << message.sequence_number << "," << message.vec[0] << "," << message.vec[1] << std::endl;
            
            _publisher->publish("PLOT", message);
            
            _sequence_number++;

        }
    
    }
    
    

    

}

