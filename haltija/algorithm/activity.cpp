#include "activity.h"
#include "debug_publisher.h"
#include <iostream>

using namespace Eigen;

ActivityDetector::ActivityDetector()
:_idx(0)
,_prob(0.999){
  
    MatrixXf Bhpf(2,1);
    MatrixXf Ahpf(2,1);
    
    Bhpf <<  0.97652981, -0.97652981;
    Ahpf <<  1.        , -0.95305962;
    
    _motion_hpf = CFilterPtr_t(new IIRFilter<MatrixXf, MatrixXcf>(Bhpf,Ahpf,1));
    
    
    MatrixXf Blpf(3,1);
    MatrixXf Alpf(3,1);
    
    //B,A = sig.iirdesign(wp=0.2/10.0,ws=1.0 / 10.0,gpass=2.0,gstop = 20.0,ftype='butter')
    Blpf <<  0.0012274,  0.0024548,  0.0012274;
    Alpf <<  1.        , -1.89848382,  0.90339341;
    
    _motion_envelope_lpf = RFilterPtr_t(new IIRFilter<MatrixXf, MatrixXf>(Blpf,Alpf,1));

}


float ActivityDetector::get_motion_prob(const Eigen::MatrixXcf & filtered_frame) {
    //highpass
    MatrixXcf hpf_sig = _motion_hpf->filter(filtered_frame);
    

    //moving average of diff^2 of last N frames
    _buf[_idx++ % ACTIVITY_DETECTOR_BUF_LEN] = hpf_sig(0,0);
    
    float accumulator = 0.0f;
    
    for (int i = 1; i < ACTIVITY_DETECTOR_BUF_LEN; i++) {
        int idx1 = (_idx + i - 1) % ACTIVITY_DETECTOR_BUF_LEN;
        int idx2 = (_idx + i) % ACTIVITY_DETECTOR_BUF_LEN;
        Complex_t dx = _buf[idx2] - _buf[idx1];
        
        accumulator += dx.real() * dx.real() + dx.imag() * dx.imag();
    }
    
    accumulator /= (float)ACTIVITY_DETECTOR_BUF_LEN;
    
    //lowpass diff^2 to compute avg value
    MatrixXf x(1,1);
    x(0,0) = accumulator;

    MatrixXf envelope_squared = _motion_envelope_lpf->filter(x);
    
    //evaluate likelihood function
    //likelihood func 1 = gaussian with variance of envelope
    //likelihood func 2 = uniform distribution with likelihood at some number
    float loglik1 = -0.25 * x(0,0) / envelope_squared(0,0);
    
    if (loglik1 < -10) {
        loglik1 = -10;
    }
    
    if (loglik1 > 10) {
        loglik1 = 10;
    }
    
    float lik1 = exp(loglik1);
    float lik2 = 0.7;
    
    
    MatrixXf p(2,1);
    p <<
    _prob,
    1.0 - _prob;
    
    MatrixXf A(2,2);
    A <<
    0.99,0.01,
    0.01,0.99;
    
    p = A * p;
    
    float total_prob = p(0,0) * lik1 + p(1,0) * lik2;
    
    _prob = p(0,0) * lik1 / total_prob;
    
    float tol = 1e-6;
    if (_prob > 1.0 - tol) {
        _prob = 1.0 - tol;
    }
    
    if (_prob < tol) {
        _prob = tol;
    }
    
    MatrixXf pm(1,1);
    pm(0,0) = _prob;
    debug_save("pm",pm);
    
    return 1.0 - _prob;
}

//float ActivityDetector::get_log_energy_change(const Complex_t & x) {
//    
//    _buf[_idx++ % ACTIVITY_DETECTOR_BUF_LEN] = x;
//    
//    float accumulator = 0.0f;
//    
//    for (int i = 1; i < ACTIVITY_DETECTOR_BUF_LEN; i++) {
//        int idx1 = (_idx + i - 1) % ACTIVITY_DETECTOR_BUF_LEN;
//        int idx2 = (_idx + i) % ACTIVITY_DETECTOR_BUF_LEN;
//        Complex_t dx = _buf[idx2] - _buf[idx1];
//        
//        accumulator += dx.real() * dx.real() + dx.imag() * dx.imag();
//    }
//    
//    accumulator /= (float)ACTIVITY_DETECTOR_BUF_LEN;
//    
//    float dlogenergy = 10.0f*log10(_last_accumulation) - 10.0f*log10(accumulator);
//    
//    _last_accumulation = accumulator;
//    
//    return 10.0f*log10(accumulator);
//}
