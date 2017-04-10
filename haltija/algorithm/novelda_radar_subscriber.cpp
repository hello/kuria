#include "novelda_radar_subscriber.h"
#include <iostream>
#include "respiration.h"
#include "log.h"
#include <unistd.h>
#include "preprocessorIIR.h"
#include "debug_publisher.h"

#define EXPECTED_SAMPLE_RATE_HZ (20)
#define NUM_FRAMES_IN_SEGMENT (20 * EXPECTED_SAMPLE_RATE_HZ)
#define NUM_FRAMES_TO_WAIT (20 * EXPECTED_SAMPLE_RATE_HZ)

#ifndef HOST_NAME_MAX
#define HOST_NAME_MAX (1024)
#endif

#ifndef LOGIN_NAME_MAX
#define LOGIN_NAME_MAX (1024)
#endif

using namespace Eigen;


NoveldaRadarSubscriber::NoveldaRadarSubscriber(RadarResultPublisherInterface * publisher,DebugPublisherInterface * debug_publisher)
:_publisher(publisher)
,_debug_publisher(debug_publisher)
,_sequence_number(0)
,_received_number(0) {
    _preprocessor = PreprocessorPtr_t(NULL);
    
    for (int i = 12; i < 38; i++) {
        _rangebins_we_care_about.insert(i);
    }
    
    DebugPublisher::initialize(debug_publisher);
    
}

NoveldaRadarSubscriber::~NoveldaRadarSubscriber() {
    DebugPublisher::deinitialize();
    
    if (_publisher) {
        delete _publisher;
        _publisher = NULL;
    }
}


void NoveldaRadarSubscriber::receive_message(const NoveldaData_t & message) {
    
    if (!_preprocessor.get()) {
        //TODO configure this from constructor
        
        _preprocessor = PreprocessorIIR::createWithDefaultHighpassFilterAndLowpass(message.range_bins.size(), NUM_FRAMES_IN_SEGMENT, NUM_FRAMES_TO_WAIT,1e-6);
        
        LOG("initialized preprocessor");
    }
    
    if (_received_number % 100 == 0) {
        LOG("I am alive: %lld frames",_received_number);
    }
    _received_number++;

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
        std::cout << "got new segment...." << std::endl;
        _combiner.set_latest_segment(segment, _rangebins_we_care_about);
    
        debug_save("segment",segment);
    }
    
    MatrixXcf transformed_frame;
    if (_combiner.get_latest_reduced_measurement(filtered_frame, transformed_frame)) {

        debug_save("transformed_frames",transformed_frame);
        
        Eigen::MatrixXf foo;
        if (_peakfinder.isPeak(transformed_frame, _received_number, foo)) {
            std::cout << "PEAK!" << std::endl;
        }
        
        //send frame over the wire, or process it or something
        
        if (_publisher) {

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
            
            //std::cout << "publish:" << message.sequence_number << "," << message.vec[0] << "," << message.vec[1] << std::endl;
            
            _publisher->publish("PLOT", message);
            
            _sequence_number++;

        }
    
    }
    
    

    

}

