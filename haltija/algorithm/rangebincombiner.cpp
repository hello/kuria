#include "rangebincombiner.h"
#include "pca.h"
#include "log.h"
#include "respiration_classifier.h"
#include "debug_publisher.h"
#include <iostream>
#include <cmath>
using namespace Eigen;

#define MAX_SCALE (1000.0f)
#define FRACTION_OF_VARIANCE_THRESHOLD (0.05)


static void print_top_modes(const Pca<MatrixXcf> & pca, const float threshold) {
    MatrixXcf principal_components = pca.get_principal_components();
    MatrixXcf eigen_vectors = pca.get_transform();
    
    principal_components.array() /= principal_components.sum();
    
    for (int i = 0 ; i < eigen_vectors.cols(); i++) {
        
        if (principal_components(principal_components.rows() - i - 1).real() < threshold) {
            break;
        }
        
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
    
}


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
    
    print_top_modes(pca,FRACTION_OF_VARIANCE_THRESHOLD);
    
    MatrixXcf eigen_vectors = pca.get_transform();

    
    /////// try and classify if respiration is possible ////////
    MatrixXcf high_variance_signals = pca.get_most_significant_signals(subset, FRACTION_OF_VARIANCE_THRESHOLD);
    
    std::vector<RespirationStats> statsvec;
    
    //pick the latest (i.e. most variance) bin that could be respiration
    int ibin = -1;
    for (int i = 0; i < high_variance_signals.cols(); i++) {
        if (!RespirationClassifier::get_respiration_stats(high_variance_signals.col(i), 20.0).is_possible_respiration) {
            continue;
        }
        
        ibin = subset.cols() - high_variance_signals.cols() + i;
    }
    
    if (ibin == -1) {
        ibin = subset.cols() - 1;
        LOG("NO POSSIBLE RESPIRATION, USING DEFAULT");
    }
    ////////////////////////////////
    
    //get last column, which is associated with the maximum variance principle component
    VectorXcf max_vector2 = eigen_vectors.col(ibin).conjugate();
    
    //if the range bins changed at all, start over
    if (!is_bins_unchanged) {
        LOG("INIT MAX MODE");
        _max_vector = max_vector2;
        _is_ready = true;
    }
    
    Complex_t val = max_vector2.dot(_max_vector.conjugate());
    
    if (val.real()*val.real() + val.imag() * val.imag() < 0.25) {
        LOG("CHANGE OF MAX MODE");
        _max_vector = max_vector2;
        _is_ready = true;
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


