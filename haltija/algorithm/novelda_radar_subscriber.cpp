#include "novelda_radar_subscriber.h"
#include <iostream>
#include "respiration.h"
#include "log.h"

#define EXPECTED_SAMPLE_RATE_HZ (20)
#define NUM_FRAMES_IN_SEGMENT (20 * EXPECTED_SAMPLE_RATE_HZ)
#define NUM_FRAMES_TO_WAIT (20 * EXPECTED_SAMPLE_RATE_HZ)


using namespace Eigen;


NoveldaRadarSubscriber::NoveldaRadarSubscriber(RadarResultPublisherInterface * publisher)
:_publisher(publisher){
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
            
            //_publisher->publish(<#const char *prefix#>, <#const RadarMessage_t &message#>)
        }
    
    }

    

}

