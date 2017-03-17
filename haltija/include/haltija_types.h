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



typedef std::complex<double> Complex128_t;
typedef std::vector<Complex128_t> Complex128Vec_t;

typedef struct  {
    Complex128Vec_t range_bins;
    uint32_t frame_id;
    bool is_base_band;
} NoveldaData_t ;


#endif //_HALTIJA_TYPES_H_
