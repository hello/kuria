#include "activity.h"

ActivityDetector::ActivityDetector()
:_idx(0)
,_last_accumulation(0.0f){
    
}


float ActivityDetector::get_log_energy_change(const Complex_t & x) {
    
    _buf[_idx++ % ACTIVITY_DETECTOR_BUF_LEN] = x;
    
    float accumulator = 0.0f;
    
    for (int i = 1; i < ACTIVITY_DETECTOR_BUF_LEN; i++) {
        int idx1 = (_idx + i - 1) % ACTIVITY_DETECTOR_BUF_LEN;
        int idx2 = (_idx + i) % ACTIVITY_DETECTOR_BUF_LEN;
        Complex_t dx = _buf[idx2] - _buf[idx1];
        
        accumulator += dx.real() * dx.real() + dx.imag() * dx.imag();
    }
    
    accumulator /= (float)ACTIVITY_DETECTOR_BUF_LEN;
    
    float dlogenergy = 10.0f*log10(_last_accumulation) - 10.0f*log10(accumulator);
    
    _last_accumulation = accumulator;
    
    return 10.0f*log10(accumulator);
}
