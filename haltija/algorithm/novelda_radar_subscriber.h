#ifndef _RADARSUBSCRIBER_H_
#define _RADARSUBSCRIBER_H_

#include "haltija_types.h"
#include "preprocessor_interface.h"
#include "rangebincombiner.h"
#include "radar_result_publisher_interface.h"
#include "filters.h"
#include "peakFinding.h"
#include "debug_publisher_interface.h"
#include "respiration_segmenter.h"
#include "activity.h"

struct NoveldaRadarSubsciberConfig {
    int min_range_bin;
    int max_range_bin;
};


class NoveldaRadarSubscriber {
public:
    NoveldaRadarSubscriber(const NoveldaRadarSubsciberConfig & config, RadarResultPublisherInterface * publisher,DebugPublisherInterface * debug_publisher);
    ~NoveldaRadarSubscriber();
        
    void receive_message(const NoveldaData_t & message);
    
private:
    PreprocessorPtr_t _preprocessor;
    PreprocessorPtr_t _pos;
    PreprocessorPtr_t _neg;

    RangebinCombiner _combiner;
    IntSet_t _rangebins_we_care_about;

    RadarResultPublisherInterface * _publisher;
    
    int64_t _sequence_number;
    int64_t _received_number;
    int64_t _stats_number;
    int64_t _modes_number;

    void publish_vec(const std::string & channel, const std::string & id, const FloatVec_t & featvec, const int sequence_number);
    
    typedef std::shared_ptr<IIRFilter<Eigen::MatrixXf, Eigen::MatrixXcf>> FilterPtr_t;
    
    RespirationSegmenter _segmenter;
    
    ActivityDetector _activity;
};

#endif //_RADARSUBSCRIBER_H_
