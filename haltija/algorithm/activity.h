#ifndef _ACTIVITY_H_
#define _ACTIVITY_H_

#include <Eigen/Core>
#include "haltija_types.h"
#include <array>

#define ACTIVITY_DETECTOR_BUF_LEN (20)

class ActivityDetector {
    
public:
    ActivityDetector();
    
    float get_log_energy_change(const Complex_t & x);
    
private:
    Eigen::MatrixXcf _last_baseband;
    std::array<Complex_t,ACTIVITY_DETECTOR_BUF_LEN> _buf;
    uint32_t _idx;
    
    float _last_accumulation;
};

#endif //_ACTIVITY_H_
