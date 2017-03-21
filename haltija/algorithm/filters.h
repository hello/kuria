#ifndef _FILTERS_H_
#define _FILTERS_H_

#include <Eigen/Core>

template <class C, class T>
class IIRFilter {
public:
    IIRFilter(const C & B, const C & A, const size_t num_bins) {
        _input_history = T::Zero(B.rows(),num_bins);
        _output_history = T::Zero(A.rows(),num_bins);
        _time_index = 0;
        
        _B = B;
        _A = A;
        
    }
    
    T filter(const T & x) {
        _input_history.row(_time_index) = x;
        
        size_t short_index = (_time_index + 1) % _B.rows();
        
        T sum = _input_history.row(_time_index) * _B(0,0);

        
        //increasing short_index means going from oldest to newest samples
        for (size_t idelay = _B.rows() - 1; idelay > 0; idelay--) {
            sum += _input_history.row(short_index) * _B(idelay,0);
            sum -= _output_history.row(short_index) * _A(idelay,0);
            short_index = (short_index + 1) % _B.rows();
        }
        
        sum *= (1.0 / _A(0,0));
        
        _output_history.row(_time_index) = sum;
        
        _time_index = (_time_index + 1) % _B.rows();
        
        return sum;
 
    }
    
    void reset() {
        _input_history.setZero();
        _output_history.setZero();
        _time_index = 0;
    }
    
private:
    T _input_history;
    T _output_history;
    C _B;
    C _A;
    size_t _time_index;
};

template <class T,class R>
R iir_filter_columns_stateful(const T & B,const T & A,
                              R & input_history, R & output_history,
                              const R & x) {
 
    
    
}

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
