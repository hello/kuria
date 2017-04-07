#include "rangebincombiner.h"
#include "pca.h"
#include "log.h"

using namespace Eigen;

RangebinCombiner::RangebinCombiner()
:_is_ready(false) {
}

RangebinCombiner::~RangebinCombiner() {
    
}


//triggers PCA computation
void RangebinCombiner::set_latest_segment(const Eigen::MatrixXcf & baseband_segment,const IntSet_t & bins_we_care_about) {
    
    /* Determine if the range bins we care about has changed */
    bool is_bins_unchanged = false;
    if (bins_we_care_about.size() == _bins_we_care_about.size()) {
        {
            auto it1 = bins_we_care_about.begin();
            auto it2 = _bins_we_care_about.begin();
         
            while(it1 != bins_we_care_about.end() && it2 != _bins_we_care_about.end()) {
                
                if ((*it1) != (*it2)) {
                    break;
                }
                
                it1++;
                it2++;
            }
            
            is_bins_unchanged = true;
        }
    }
    
    _bins_we_care_about = bins_we_care_about;

    /* do principal component analysis on the subset of range bins that we care about */

    const MatrixXcf subset = get_subset(baseband_segment,bins_we_care_about);
    
    
    MatrixXcf principal_components;
    MatrixXcf eigen_vectors;
    MatrixXcf transformed_values;
    
    if (!pca(subset,principal_components,eigen_vectors,transformed_values)) {
        LOG("pca failed for some reason");
        return;
    }

    //get last column, which is associated with the maximum variance principle component
    VectorXcf max_vector2 = eigen_vectors.col(eigen_vectors.cols()-1).conjugate();
    
    //if the range bins changed at all, start over
    if (!is_bins_unchanged) {
        LOG("INIT MAX EIGEN VECTOR");
        _max_vector = max_vector2;
        _is_ready = true;
        get_max_rangebin();
    }
    
    Complex_t val = max_vector2.dot(_max_vector.conjugate());
    
    if (val.real()*val.real() + val.imag() * val.imag() < 0.25) {
        LOG("CHANGE OF MAX EIGEN VECTOR");
        _max_vector = max_vector2;
        _is_ready = true;
        get_max_rangebin();
    }
}

//returns false if transformation is unavailable.
//transformed frame is complex measurement of size 1x1
bool RangebinCombiner::get_latest_reduced_measurement(const Eigen::MatrixXcf & baseband_frame,Eigen::MatrixXcf & transformed_frame) {
    
    if (!_is_ready) {
        return false;
    }
 
    const MatrixXcf subset = get_subset(baseband_frame, _bins_we_care_about);
    
    transformed_frame = subset * _max_vector;
    
    return true;
}


Eigen::MatrixXcf RangebinCombiner::get_subset(const Eigen::MatrixXcf & baseband_segment,const IntSet_t & bins_we_care_about) const {
    
    MatrixXcf subset(baseband_segment.rows(),bins_we_care_about.size());
    size_t icol = 0;
    for (auto it = bins_we_care_about.begin(); it != bins_we_care_about.end(); it++) {
        subset.col(icol) = baseband_segment.col(*it);
        icol++;
    }

    return subset;
    
    
}

int RangebinCombiner::get_max_rangebin() const {
    
    //print max range bin
    float max = 0.0;
    int imax = 0;
    for (int i = 0; i < _max_vector.rows(); i++) {
        Complex_t v = _max_vector(i,0);
        float a = v.real() * v.real() + v.imag() * v.imag();
        if (a > max) {
            imax = i;
            max = a;
        }
    }
    
    int index = 0;
    int real_range_bin = -1;
    for (auto it = _bins_we_care_about.begin(); it != _bins_we_care_about.end(); it++) {
        if (index == imax) {
            real_range_bin = *it;
            break;
        }
        
        index++;
    }
    
    LOG("max eigenvector in bin %d",real_range_bin);

    return real_range_bin;
}
