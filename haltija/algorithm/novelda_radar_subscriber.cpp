#include "novelda_radar_subscriber.h"
#include <iostream>
#include "respiration.h"
#include "log.h"

#define EXPECTED_SAMPLE_RATE_HZ (20)
#define NUM_FRAMES_IN_SEGMENT (20 * EXPECTED_SAMPLE_RATE_HZ)
#define NUM_FRAMES_TO_WAIT (20 * EXPECTED_SAMPLE_RATE_HZ)


NoveldaRadarSubscriber::NoveldaRadarSubscriber() {
    _preprocessor = PreprocessorPtr_t(NULL);
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
       
    Eigen::MatrixXcf filtered_frame;
    Eigen::MatrixXcf segment;
    
    uint32_t flags = _preprocessor->add_frame(frame, filtered_frame, segment);
    
    //DO KALMAN FILTERS HERE
    if (flags & PREPROCESSOR_FLAGS_FRAME_READY) {
        
    }
    
    if (flags & PREPROCESSOR_FLAGS_SEGMENT_READY) {
        //DO FRAME PROCESSING HERE
        
        
        Eigen::MatrixXf features = get_respiration_features(segment);
        
    }
    

}

