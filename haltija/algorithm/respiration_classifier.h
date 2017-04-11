#ifndef _RESPIRATIONCLASSIFIER_H_
#define _RESPIRATIONCLASSIFIER_H_

#include <Eigen/Core>

struct RespirationStats  {
    
    RespirationStats(const float mean, const float stddev)
    :peak_to_peak_mean_seconds(mean)
    ,peak_to_peak_stddev_seconds(stddev)
    ,is_valid(true){}
    
    RespirationStats()
    :peak_to_peak_mean_seconds(0)
    ,peak_to_peak_stddev_seconds(0)
    ,is_valid(false){}
    
    const float peak_to_peak_mean_seconds;
    const float peak_to_peak_stddev_seconds;
    const bool is_valid;
} ;


class RespirationClassifier {
public:
    static int is_respiration(const Eigen::MatrixXcf & range_bins_of_interest, const float sample_rate_hz);
    static RespirationStats get_respiration_stats(const Eigen::MatrixXcf & probable_respiration_linear_combination,const float sample_rate_hz);
    
};

#endif //_RESPIRATIONCLASSIFIER_H_
