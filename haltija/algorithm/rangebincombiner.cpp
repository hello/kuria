#include "rangebincombiner.h"
#include "pca.h"
#include "log.h"
#include "respiration_classifier.h"
#include "debug_publisher.h"
#include <iostream>
#include <cmath>
using namespace Eigen;

#define MAX_SCALE (1000.0f)

RangebinCombiner::RangebinCombiner()
:_is_ready(false) {
}

RangebinCombiner::~RangebinCombiner() {
    
}

//scale data to attenuate near signals
Eigen::MatrixXcf RangebinCombiner::normalize_by_free_space_loss(const Eigen::MatrixXcf & baseband_segment) const {
    
    Eigen::MatrixXcf scaled_baseband = baseband_segment;
    
    int idx = 0;
    const float begin_bin = (float)*_bins_we_care_about.begin();
    for (auto it = _bins_we_care_about.begin(); it != _bins_we_care_about.end(); it++) {
        float scale = pow(begin_bin / (float)(*it),-4.0) / MAX_SCALE;
        
        if (scale > 1.0) {
            scale = 1.0;
        }
        
        
        scaled_baseband.col(idx).array() *= scale;
        
        idx++;
    }
    
    return scaled_baseband;
    
}

//triggers PCA computation
Eigen::MatrixXcf RangebinCombiner::set_latest_segment(const Eigen::MatrixXcf & baseband_segment,const IntSet_t & bins_we_care_about) {
    
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

    const MatrixXcf scaled_baseband = normalize_by_free_space_loss(baseband_segment);
    
    const MatrixXcf subset = get_subset(scaled_baseband,bins_we_care_about);
    
    
    Pca<MatrixXcf> pca;
    
    pca.fit(subset);
    
    MatrixXcf principal_components = pca.get_principal_components();
    MatrixXcf eigen_vectors = pca.get_transform();
    MatrixXcf transformed_values = pca.get_transformed_values(subset);

    principal_components.array() /= principal_components.sum();
    
    for (int i = 0 ; i < 5; i++) {
        auto this_col = eigen_vectors.col(eigen_vectors.cols() - i - 1);
        auto magnitudes = this_col.array().real() * this_col.array().real() +  this_col.array().imag()*this_col.array().imag();
        //std::cout << magintudes.transpose() << std::endl;
        //std::cout << myvec.transpose() << std::endl;
        float sum = 0.0;
        int significant_magnitudes = 0;
        for (int j = 0; j < magnitudes.rows(); j++) {
            sum += magnitudes(j) * j;
            if (magnitudes(j) > 0.05) {
                significant_magnitudes++;
            }
        }
        
        std::cout << principal_components(principal_components.rows() - i - 1).real() << "," << sum << "," << significant_magnitudes <<std::endl;
    }
    
    
    int ibin = transformed_values.cols() - 1;
    
    //get last column, which is associated with the maximum variance principle component
    VectorXcf max_vector2 = eigen_vectors.col(ibin).conjugate();
    
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
    
    return pca.get_most_significant_signals(subset, 1e-1);
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
