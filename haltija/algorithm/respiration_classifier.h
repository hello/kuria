#ifndef _RESPIRATIONCLASSIFIER_H_
#define _RESPIRATIONCLASSIFIER_H_

#include <Eigen/Core>

typedef struct  {
    float peak_to_peak_stddev;
    float peak_to_peak_mean;
    bool is_valid;
} RespirationStats_t ;


class RespirationClassifier {
public:
    static int is_respiration(const Eigen::MatrixXcf & range_bins_of_interest, const float sample_rate_hz);
    static bool get_respiration_stats(const Eigen::MatrixXcf & probable_respiration_linear_combination, RespirationStats_t & result);
    
};

#endif //_RESPIRATIONCLASSIFIER_H_
