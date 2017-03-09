#include "respiration.h"
using namespace Eigen;

IntVec_t get_possible_respiration_range_bins(const MatrixXcf & filtered_baseband) {
    IntVec_t possible_range_bins;
    
    //return indices of all range bins
    //TODO put in large motion determination logic, and no motion logic
    for (int i = 0; i < filtered_baseband.cols(); i++) {
        possible_range_bins.push_back(i);
    }
    
    return possible_range_bins;
    
}

MatrixXf get_respiration_features(const MatrixXcf & filtered_baseband, const IntVec_t & possible_range_bins) {
    //given possible range bins, do PCA or something.  Or pick a range bin.
    return MatrixXf::Zero(NUM_RESPIRATION_FEATURES,1);
}
