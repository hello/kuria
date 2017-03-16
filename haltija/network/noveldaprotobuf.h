#ifndef _NOVELDAPROTOBUF_H_
#define _NOVELDAPROTOBUF_H_

#include <complex>
#include <vector>


typedef std::complex<double> Complex128_t;
typedef std::vector<Complex128_t> Complex128Vec_t;

struct NoveldaData {
    Complex128Vec_t range_bins;
    uint32_t frame_id;
    bool is_base_band;
};

class NoveldaProtobuf {
public:
    bool deserialize_protobuf(const uint8_t * protobuf_bytes, const size_t protobuf_size, NoveldaData & deserialized_data);
    
};





#endif
