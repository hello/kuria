#include "haltijamath.h"
#include <string.h>
#include "kiss_fft.h"

typedef std::complex<float> Complex_t;

bool HaltijaMath::complex_fft(const int nfft, const Eigen::VectorXcf & x, Eigen::VectorXcf & output) {
    //COMPLETELY UNTESTED -- go write a unit test?
    kiss_fft_cpx xin[nfft];
    kiss_fft_cpx xout[nfft];
    
    //set input buffer to zero, c-style
    memset(xin,0,sizeof(xin));
    
    
    //pick the smaller of nfft or the input
    const int N = x.size() < nfft ? x.size() : nfft;
    
    for (int i = 0; i < N; i++) {
        xin[i].r = x(i).real();
        xin[i].i = x(i).imag();
    }
    
    
    kiss_fft_cfg mycfg = kiss_fft_alloc(nfft,0,NULL,NULL);
    
    kiss_fft(mycfg,xin,xout);
    
    //copy out result
    Eigen::VectorXcf outvec(nfft);
    
    for (int i = 0; i < nfft; i++) {
        outvec(i) = Complex_t(xout[i].r,xout[i].i);
    }
    
    kiss_fft_free(mycfg);

    output = outvec;
    
    return true;
    
}
