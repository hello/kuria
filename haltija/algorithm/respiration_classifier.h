#ifndef _RESPIRATIONCLASSIFIER_H_
#define _RESPIRATIONCLASSIFIER_H_

#include <Eigen/Core>

#include "haltija_types.h"



struct RespirationStats  {
    
    RespirationStats(const float mean, const float stddev,const float the_energy_db,bool respiration)
    :peak_to_peak_mean_seconds(mean)
    ,peak_to_peak_stddev_seconds(stddev)
    ,energy_db(the_energy_db)
    ,is_valid(true)
    ,is_possible_respiration(respiration)
    {}
    
    RespirationStats()
    :peak_to_peak_mean_seconds(0)
    ,peak_to_peak_stddev_seconds(0)
    ,energy_db(-INFINITY)
    ,is_valid(false)
    ,is_possible_respiration(false) {}
    
    const float peak_to_peak_mean_seconds;
    const float peak_to_peak_stddev_seconds;
    const float energy_db;
    const bool is_valid;
    const bool is_possible_respiration;
} ;




class RespirationClassifier {
public:
    static RespirationStats get_respiration_stats(const Eigen::MatrixXcf & probable_respiration_linear_combination,const float sample_rate_hz);
    
    static Eigen::MatrixXf get_bandpassed_and_reduced_signal(const Eigen::MatrixXcf & probable_respiration_linear_combinations);
    

    
};

#endif //_RESPIRATIONCLASSIFIER_H_
