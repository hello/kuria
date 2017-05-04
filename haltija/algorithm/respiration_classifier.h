#ifndef _RESPIRATIONCLASSIFIER_H_
#define _RESPIRATIONCLASSIFIER_H_

#include <Eigen/Core>

#include "haltija_types.h"

enum {
    exhaled = 0,
    exhaling,
    inhaled,
    inhaling,
    NUM_RESPIRATION_STATES
} ERespirationState_t;



struct RespirationStats  {
    
    RespirationStats(const float mean, const float stddev,bool respiration,Complex_t r[NUM_RESPIRATION_STATES])
    :peak_to_peak_mean_seconds(mean)
    ,peak_to_peak_stddev_seconds(stddev)
    ,is_valid(true)
    ,is_possible_respiration(respiration)
    ,resipration_clusters{r[0],r[1],r[2],r[3]} {}
    
    RespirationStats()
    :peak_to_peak_mean_seconds(0)
    ,peak_to_peak_stddev_seconds(0)
    ,is_valid(false)
    ,is_possible_respiration(false) {}
    
    const float peak_to_peak_mean_seconds;
    const float peak_to_peak_stddev_seconds;
    const bool is_valid;
    const bool is_possible_respiration;
    const Complex_t resipration_clusters[NUM_RESPIRATION_STATES];
} ;




class RespirationClassifier {
public:
    static RespirationStats get_respiration_stats(const Eigen::MatrixXcf & probable_respiration_linear_combination,const float sample_rate_hz);
    
};

#endif //_RESPIRATIONCLASSIFIER_H_
