#ifndef _FILTERS_H_
#define _FILTERS_H_

#include <Eigen/Core>

//B is filter coefficient, x is matrix, icol specifies the column on which to operate
template <class T>
T fir_filter_columns(const T & B, const T & x, size_t icol = 0) {
    
    const size_t ntaps = B.rows();
    const size_t vec_len = x.rows();
    const int N = vec_len - ntaps + 1;
    
    if (ntaps > vec_len) {
        return T();
    }
    
    T outmat(N,1);
    
    for (int iout = 0; iout < N; iout++) {
        T foo = x.block(iout,0,ntaps,1);
        outmat(iout) = (B.array() * x.block(iout,icol,ntaps,1).array()).sum();
    }
    
    return outmat;
}

//B and A are filter coefficient, x is matrix, icol specifies the column on which to operate
/*
template <class T>
T iir_filter_columns(const T & B, const T & A, const T & x, size_t icol = 0) {

    const size_t bsize = B.rows();
    const size_t asize = A.rows();

    const size_t vec_len = x.rows();
    
    if (ntaps > vec_len) {
        return T();
    }
    
    T outmat(N,1);
    
    for (int iout = 0; iout < N; iout++) {
        T foo = x.block(iout,0,ntaps,1);
        outmat(iout) = (B.array() * x.block(iout,icol,ntaps,1).array()).sum();
    }
    
    return outmat;
 
}
 */

#endif
