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


static FloatMatrix_t compute_top_modes(const Pca<MatrixXcf> & pca, const float threshold, bool print = true) {
    MatrixXcf principal_components = pca.get_principal_components();
    const MatrixXcf eigen_vectors = pca.get_transform();
    
    
    FloatMatrix_t important_magnitudes;
    
    principal_components.array() /= principal_components.sum();

    for (int i = 0 ; i < eigen_vectors.cols(); i++) {
        
        if (principal_components(principal_components.rows() - i - 1).real() < threshold) {
            break;
        }
        
        MatrixXcf this_col = eigen_vectors.col(eigen_vectors.cols() - i - 1);

        MatrixXf magnitudes = this_col.array().real() * this_col.array().real() +  this_col.array().imag()*this_col.array().imag();

        float sum = 0.0;
        int significant_magnitudes = 0;
        for (int j = 0; j < magnitudes.rows(); j++) {
            sum += magnitudes(j) * j;
            if (magnitudes(j) > 0.05) {
                significant_magnitudes++;
            }
        }
        
        
        
        if (print) {
            std::cout << principal_components(principal_components.rows() - i - 1).real() << "," << sum << "," << significant_magnitudes <<std::endl;
        }
        
        FloatVec_t v;
        v.reserve(magnitudes.rows());
        v.insert(v.begin(),magnitudes.data(),magnitudes.data() + magnitudes.rows());
        important_magnitudes.push_back(v);
        
    }
    
    return important_magnitudes;
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
        
        //std::cout << *it << "," << scale << std::endl;
        
        if (scale > 1.0) {
            scale = 1.0;
        }
        
        
        scaled_baseband.col(idx).array() *= scale;
        
        idx++;
    }
    
    return scaled_baseband;
    
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

    const MatrixXcf scaled_baseband = normalize_by_free_space_loss(baseband_segment);
    
    const MatrixXcf subset = get_subset(scaled_baseband,bins_we_care_about);
    
    
    Pca<MatrixXcf> pca;
    
    pca.fit(subset);
    
    _top_modes = compute_top_modes(pca,FRACTION_OF_VARIANCE_THRESHOLD);
    
    MatrixXcf eigen_vectors = pca.get_transform();

    
    /////// try and classify if respiration is possible ////////
    MatrixXcf high_variance_signals = pca.get_most_significant_signals(subset, FRACTION_OF_VARIANCE_THRESHOLD);
    
    std::vector<RespirationStats> statsvec;
    
    //pick the latest (i.e. most variance) bin that could be respiration
    int ibin = -1;
    for (int i = 0; i < high_variance_signals.cols(); i++) {
        _best_respiration_segment = high_variance_signals.col(i);

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
    
    MatrixXf magnitude_vec2 =
    max_vector2.real().array() * max_vector2.real().array() +
    max_vector2.imag().array() * max_vector2.imag().array();
    
    magnitude_vec2.array() -= magnitude_vec2.sum() / (float)magnitude_vec2.rows();
    
    MatrixXf magnitude_vec =
    _max_vector.real().array() * _max_vector.real().array() +
    _max_vector.imag().array() * _max_vector.imag().array();

    
    magnitude_vec.array() -= magnitude_vec.sum() / (float)magnitude_vec.rows();

    MatrixXf modematrix(magnitude_vec.rows(),2);
    modematrix.col(0) = magnitude_vec;
    modematrix.col(1) = magnitude_vec2;
    
    MatrixXf covmat = modematrix.transpose() * modematrix;
    
    
    
    float correlation = 1.0;
    
    if (covmat(0,0) > 1e-6 && covmat(1,1) > 1e-6) {
        correlation = covmat(0,1) / sqrt(covmat(1,1)) / sqrt(covmat(0,0));
    }
    

    //float similarity = val.real()*val.real() + val.imag() * val.imag();
    std::cout << "correlation: " << correlation << std::endl;
    if (correlation < 0.6) {
        LOG("CHANGE OF MAX MODE");
        _max_vector = max_vector2;
        _is_ready = true;
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


const FloatMatrix_t & RangebinCombiner::get_top_modes() const {
    return _top_modes;
}
const Eigen::MatrixXcf & RangebinCombiner::get_best_respiration_segment() const {
    return _best_respiration_segment;
}
