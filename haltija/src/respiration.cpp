#include "respiration.h"
#include "haltijamath.h"

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






MatrixXf get_respiration_features(const MatrixXcf & filtered_baseband) {
    const IntVec_t possible_range_bins = get_possible_respiration_range_bins(filtered_baseband);

    //assemble matrix, copy columns over from selected range bins
    MatrixXcf selected_range_bins_baseband = MatrixXcf::Zero(filtered_baseband.rows(),possible_range_bins.size());
    
    for (int isel = 0; isel < possible_range_bins.size(); isel++) {
        selected_range_bins_baseband.col(isel) = filtered_baseband.col(possible_range_bins[isel]);
    }
    
    //get reals by 2x2 PCA
    MatrixXf reals = HaltijaMath::project_complex_cols_into_reals(selected_range_bins_baseband);

    //TODO respiration detection?????!!?#@
    //count peaks / zero crossings, get timing, etc.
    
    
    //given possible range bins, do PCA or something.  Or pick a range bin.
    return MatrixXf::Zero(NUM_RESPIRATION_FEATURES,1);
}
