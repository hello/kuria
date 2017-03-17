#ifndef _NOVELDAPROTOBUF_H_
#define _NOVELDAPROTOBUF_H_

#include <complex>
#include <vector>
#include "haltija_types.h"

class NoveldaProtobuf {
public:
    bool deserialize_protobuf(const uint8_t * protobuf_bytes, const size_t protobuf_size, NoveldaData_t & deserialized_data);
    
};





#endif
