#ifndef _FILTERS_H_
#define _FILTERS_H_

#include <Eigen/Core>

//B is filter coefficient, x is matrix, icol specifies the column on which to operate
template <class T,class R>
R fir_filter_columns(const T & B, const R & x) {
    
    const size_t ntaps = B.rows();
    const size_t vec_len = x.rows();
    const int N = vec_len - ntaps + 1;
    
    if (ntaps > vec_len) {
        return T();
    }
    
    R outmat(N,x.cols());
    
    for (int iout = 0; iout < N; iout++) {
        for (int icol = 0; icol < x.cols(); icol++) {
            outmat(iout,icol) = (B.array() * x.block(iout,icol,ntaps,1).array()).sum();
        }
    }
    
    return outmat;
}

template <class T>
T circular_shift_columns(const T & x, int shift) {
    T xshifted(x.rows(),x.cols());

    shift = (shift + x.rows()) % x.rows();
    
    int n1 = x.rows() - shift;
    int n2 = x.rows() - n1;
        
    xshifted.block(shift,0,n1,x.cols()) = x.block(0,0,n1,x.cols());
    xshifted.block(0,0,n2,x.cols()) = x.block(n1,0,n2,x.cols());
    
    return xshifted;
 
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
