#ifndef _ACTIVITY_H_
#define _ACTIVITY_H_

#include <Eigen/Core>
#include "haltija_types.h"
#include <array>
#include "filters.h"

#define ACTIVITY_DETECTOR_BUF_LEN (20)

class ActivityDetector {
    
public:
    ActivityDetector();
        
    float get_motion_prob(const Eigen::MatrixXcf & filtered_frame);
    
private:
    Eigen::MatrixXcf _last_baseband;
    std::array<Complex_t,ACTIVITY_DETECTOR_BUF_LEN> _buf;
    uint32_t _idx;
    typedef std::shared_ptr<IIRFilter<Eigen::MatrixXf, Eigen::MatrixXcf>> CFilterPtr_t;
    typedef std::shared_ptr<IIRFilter<Eigen::MatrixXf, Eigen::MatrixXf>> RFilterPtr_t;

    CFilterPtr_t _motion_hpf;
    RFilterPtr_t _motion_envelope_lpf;

    float _prob;
};

#endif //_ACTIVITY_H_
