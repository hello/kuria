#ifndef _RADARSUBSCRIBER_H_
#define _RADARSUBSCRIBER_H_

#include "haltija_types.h"
#include "preprocessor.h"

class NoveldaRadarSubscriber {
public:
    NoveldaRadarSubscriber();
    ~NoveldaRadarSubscriber();
    
    void receive_message(const NoveldaData_t & message);
    
private:
    PreprocessorPtr_t _preprocessor;
};

#endif //_RADARSUBSCRIBER_H_
