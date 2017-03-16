#ifndef _HALTIJAMATH_H_
#define _HALTIJAMATH_H_

#include <Eigen/Core>
#include "kiss_fft.h"
#include "haltija_types.h"

namespace HaltijaMath {
    
    
//templatized so this will work on real as well as complex data
    
template <class T>
bool fft(const int nfft, const T & x, Eigen::MatrixXcf & output) {
    //COMPLETELY UNTESTED -- go write a unit test?
    kiss_fft_cpx xin[nfft];
    kiss_fft_cpx xout[nfft];
    
    //set input buffer to zero, c-style
    memset(xin,0,sizeof(xin));
    
    
    //pick the smaller of nfft or the input
    const int N = x.size() < nfft ? x.size() : nfft;
    
    for (int i = 0; i < N; i++) {
        std::complex<float> v = x(i,0);
        xin[i].r = v.real();
        xin[i].i = v.imag();
    }
    
    
    kiss_fft_cfg mycfg = kiss_fft_alloc(nfft,0,NULL,NULL);
    
    kiss_fft(mycfg,xin,xout);
    
    //copy out result
    Eigen::MatrixXcf outvec(nfft,1);
    
    for (int i = 0; i < nfft; i++) {
        outvec(i) = std::complex<float>(xout[i].r,xout[i].i);
    }
    
    kiss_fft_free(mycfg);
    
    output = outvec;
    
    return true;
    
}

//no averaging, so expect the output to be different than pwelch
template <class T>
bool psd(const int nfft, const T & x, Eigen::MatrixXf & output) {
    
    T xwindowed = x;
    
    //TODO pre-compute Hann window coefficients
    //apply Hann window
    const size_t N = x.rows();
    
    for (int n = 0; n < x.rows(); n++) {
        xwindowed(n,0) *= 0.5 * (1.0 - cos(2 * M_PI * n / (N - 1) ));
    }
    
    Eigen::MatrixXcf fft_result;
    
    if (!fft(nfft,xwindowed,fft_result)) {
        return false;
    }

    output.resizeLike(fft_result);
    
    for (int i = 0; i < output.rows(); i++) {
        const float real = fft_result(i,0).real();
        const float imag = fft_result(i,0).imag();
        output(i,0) = 10*log10(real*real + imag*imag);
    }
    
    return true;
}
    
    
Eigen::MatrixXf project_complex_cols_into_reals(const Eigen::MatrixXcf & c);


}
#endif //_HALTIJAMATH_H_
