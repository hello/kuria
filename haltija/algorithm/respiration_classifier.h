#ifndef _RESPIRATIONCLASSIFIER_H_
#define _RESPIRATIONCLASSIFIER_H_

#include <Eigen/Core>

class RespirationClassifier {
public:
    static int is_respiration(const Eigen::MatrixXcf & range_bins_of_interest, const float sample_rate_hz);
    
    
};

#endif //_RESPIRATIONCLASSIFIER_H_
