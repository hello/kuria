#ifndef _HALTIJAMATH_H_
#define _HALTIJAMATH_H_

#include <Eigen/Core>

namespace HaltijaMath {
    bool complex_fft(const int nfft, const Eigen::VectorXcf & x, Eigen::VectorXcf & output);

}

#endif //_HALTIJAMATH_H_
