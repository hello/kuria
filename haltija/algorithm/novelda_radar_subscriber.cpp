#include "novelda_radar_subscriber.h"
#include <iostream>
#include "respiration.h"
#include "log.h"
#include <unistd.h>
#include "preprocessorIIR.h"
#include "debug_publisher.h"
#include "respiration_classifier.h"

#define EXPECTED_SAMPLE_RATE_HZ (20)
#define NUM_FRAMES_IN_SEGMENT (20 * EXPECTED_SAMPLE_RATE_HZ)
#define NUM_FRAMES_TO_WAIT (5 * EXPECTED_SAMPLE_RATE_HZ)

#ifndef HOST_NAME_MAX
#define HOST_NAME_MAX (1024)
#endif

#ifndef LOGIN_NAME_MAX
#define LOGIN_NAME_MAX (1024)
#endif

using namespace Eigen;


NoveldaRadarSubscriber::NoveldaRadarSubscriber(RadarResultPublisherInterface * publisher,DebugPublisherInterface * debug_publisher)
:_publisher(publisher)
,_sequence_number(0)
,_received_number(0)
,_stats_number(0)
,_modes_number(0)
,_is_respiration(false) {
    _preprocessor = PreprocessorPtr_t(NULL);
    
    for (int i = 8; i < 70; i++) {
        _rangebins_we_care_about.insert(i);
    }
    
    DebugPublisher::initialize(debug_publisher);
    
    MatrixXf Blpf(2,1);
    MatrixXf Alpf(2,1);

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
        
        RespirationStats stats = RespirationClassifier::get_respiration_stats(_combiner.get_best_respiration_segment(),EXPECTED_SAMPLE_RATE_HZ);
        
        _is_respiration = stats.is_possible_respiration;
        
        auto top_modes = _combiner.get_top_modes();
        for (auto it = top_modes.begin(); it != top_modes.end(); it++)  {
            (*it).size();
            
            publish_vec("PLOT","modes",*it,_modes_number);
        }

        _modes_number++;

        
        //publish them
        
        std::cout
        << "mean breath duration: " << stats.peak_to_peak_mean_seconds << ", "
        << "breaths per min: " << 60.0 / stats.peak_to_peak_mean_seconds << ", "
        << "std dev seconds: " << stats.peak_to_peak_stddev_seconds << ","
        << "energy (dB): " << stats.energy_db
        << std::endl;
        
        FloatVec_t statsvec;
        
        statsvec.push_back(stats.peak_to_peak_mean_seconds);
        statsvec.push_back(stats.peak_to_peak_stddev_seconds);
        statsvec.push_back(stats.energy_db);
        statsvec.push_back(stats.is_possible_respiration);
        
        publish_vec("STATS","respiration",statsvec,_stats_number);
        publish_vec("PLOT","respiration",statsvec,_stats_number);
        
        
        debug_save("segment",segment);
    }
    
    MatrixXcf transformed_frame;
    if (_combiner.get_latest_reduced_measurement(filtered_frame, transformed_frame)) {
        
        debug_save("transformed_frames",transformed_frame);
        
        
        if (flags & PREPROCESSOR_FLAGS_SEGMENT_READY) {
            
            /* if there's a new segment, segment the segment? okay our naming sucks.  Dang. */
            MatrixXcf live_signal;
            _combiner.get_latest_reduced_measurement(segment, live_signal);
            
            /* this will figure out the values of the live signal that correspond to the stages of breathing */
            _segmenter.set_segment(_combiner.get_best_respiration_segment(), live_signal,_is_respiration);
        }
        
        RespirationPrediction prediction = _segmenter.predict_respiration_state(transformed_frame,EXPECTED_SAMPLE_RATE_HZ);
        {
            Eigen::MatrixXf temp(4,1);
            temp << prediction.respiration_probs[0],prediction.respiration_probs[1],prediction.respiration_probs[2],prediction.respiration_probs[3];
            debug_save("resp_probs", temp);
        }
        
        FloatVec_t respprobs;
        for (int i = 0; i < NUM_RESPIRATION_STATES; i++) {
            respprobs.push_back(prediction.respiration_probs[i]);
        }
        publish_vec("v1/breath", "respprobs", respprobs, 0);

        
        //send frame over the wire, or process it or something
        
        FloatVec_t iqdata;
        
        iqdata.push_back(transformed_frame(0,0).real());
        iqdata.push_back(transformed_frame(0,0).imag());
        
        publish_vec("PLOT", "maxvarresp", iqdata, _sequence_number++);
        
        
    }
    
}



    



void NoveldaRadarSubscriber::publish_vec(const std::string & channel, const std::string & id, const FloatVec_t & featvec, const int sequence_number) {
    
    if (!_publisher) {
        return;
    }
    
    char hostname[HOST_NAME_MAX] = {0};
    char username[LOGIN_NAME_MAX] = {0};
    gethostname(hostname, HOST_NAME_MAX);
    getlogin_r(username, LOGIN_NAME_MAX);
    
    RadarMessage_t message;
    message.sequence_number = sequence_number;
    message.vec.insert(message.vec.begin(),featvec.begin(),featvec.end());
    message.id = id;
    message.device_id = hostname;
    
   
    _publisher->publish(channel.c_str(), message);
    
}

