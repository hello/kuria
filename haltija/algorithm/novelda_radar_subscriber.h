#ifndef _RADARSUBSCRIBER_H_
#define _RADARSUBSCRIBER_H_

#include "haltija_types.h"
#include "preprocessor.h"
#include "rangebincombiner.h"
#include "radar_result_publisher_interface.h"

class NoveldaRadarSubscriber {
public:
    NoveldaRadarSubscriber(RadarResultPublisherInterface * publisher);
    ~NoveldaRadarSubscriber();
    
    void set_publisher(RadarResultPublisherInterface * publisher);
    
    void receive_message(const NoveldaData_t & message);
    
private:
    PreprocessorPtr_t _preprocessor;
    RangebinCombiner _combiner;
    IntSet_t _rangebins_we_care_about;

    RadarResultPublisherInterface * _publisher;
    
    int64_t _sequence_number;
    int64_t _received_number;
};

#endif //_RADARSUBSCRIBER_H_
