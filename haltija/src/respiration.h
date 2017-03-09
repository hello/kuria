#ifndef _RESPIRATION_H_
#define _RESPIRATION_H_

#include <Eigen/Core>
#include <vector>
#include "haltija_types.h"


enum ERespirationFeatures_t {
    respiration_exists = 0,
    respiration_rate,
    respiration_variability,
    NUM_RESPIRATION_FEATURES
};


IntVec_t get_possible_respiration_range_bins(const Eigen::MatrixXcf & filtered_baseband);

Eigen::MatrixXf get_respiration_features(const Eigen::MatrixXcf & filtered_baseband, const IntVec_t & possible_range_bins);



#endif //_RESPIRATION_H_
