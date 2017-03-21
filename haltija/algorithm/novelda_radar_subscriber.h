#ifndef _RADARSUBSCRIBER_H_
#define _RADARSUBSCRIBER_H_

#include "haltija_types.h"
#include "preprocessor.h"
#include "rangebincombiner.h"

class NoveldaRadarSubscriber {
public:
    NoveldaRadarSubscriber();
    ~NoveldaRadarSubscriber();
    
    void receive_message(const NoveldaData_t & message);
    
private:
    PreprocessorPtr_t _preprocessor;
    RangebinCombiner _combiner;
    IntSet_t _rangebins_we_care_about;
};

#endif //_RADARSUBSCRIBER_H_
