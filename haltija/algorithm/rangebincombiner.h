#ifndef _RANGEBINCOMBINER_H_
#define _RANGEBINCOMBINER_H_

#include "haltija_types.h"
#include <Eigen/Core>



/* Does PCA to reduce the dimensions multiple baseband rangebins 
   into a single dimension. It's in a class because it has state */
class RangebinCombiner {
public:
    RangebinCombiner();
    ~RangebinCombiner();
    
    
    //triggers PCA computation
    //returns most significant columns of baseband segment
    void set_latest_segment(const Eigen::MatrixXcf & baseband_segment,const IntSet_t & bins_we_care_about);
    
    //returns false if transformation is unavailable.
    //transformed frame is complex measurement of size 1x1
    bool get_latest_reduced_measurement(const Eigen::MatrixXcf & baseband_frame,Eigen::MatrixXcf & transformed_frame);
    
    const FloatMatrix_t & get_top_modes() const;
    const Eigen::MatrixXcf & get_best_respiration_segment() const;
    
private:
    Eigen::VectorXcf _max_vector;
    bool _is_ready;
    IntSet_t _bins_we_care_about;
    
    Eigen::MatrixXcf get_subset(const Eigen::MatrixXcf & baseband_segment,const IntSet_t & bins_we_care_about) const;
    Eigen::MatrixXcf normalize_by_free_space_loss(const Eigen::MatrixXcf & baseband_segment) const;

    Eigen::MatrixXcf _best_respiration_segment;
    FloatMatrix_t _top_modes;
    
    
};


#endif //_RANGEBINCOMBINER_H_
