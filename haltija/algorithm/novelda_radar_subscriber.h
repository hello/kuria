#ifndef _RADARSUBSCRIBER_H_
#define _RADARSUBSCRIBER_H_

#include "haltija_types.h"
#include "preprocessor_interface.h"
#include "rangebincombiner.h"
#include "radar_result_publisher_interface.h"
#include "filters.h"
#include "peakFinding.h"
#include "debug_publisher_interface.h"

class NoveldaRadarSubscriber {
public:
    NoveldaRadarSubscriber(RadarResultPublisherInterface * publisher,DebugPublisherInterface * debug_publisher);
    ~NoveldaRadarSubscriber();
        
    void receive_message(const NoveldaData_t & message);
    
private:
    PreprocessorPtr_t _preprocessor;
    RangebinCombiner _combiner;
    IntSet_t _rangebins_we_care_about;

    RadarResultPublisherInterface * _publisher;
    DebugPublisherInterface * _debug_publisher; //TODO put this in a singleton or something
    
    int64_t _sequence_number;
    int64_t _received_number;
    
    IIRFilter<Eigen::MatrixXf, Eigen::MatrixXcf> * lpf;

    Peakfinder _peakfinder;
};

#endif //_RADARSUBSCRIBER_H_
