#ifndef _HALTIJA_TYPES_H_
#define _HALTIJA_TYPES_H_

#include <complex>
#include <vector>

typedef long long Timestamp_t;

typedef std::complex<float> Complex_t;
typedef std::vector<Complex_t> ComplexVec_t;
typedef std::vector<int> IntVec_t;

typedef struct {
    Timestamp_t timestamp;
    ComplexVec_t data;
} BasebandDataFrame_t ;


#endif //_HALTIJA_TYPES_H_
