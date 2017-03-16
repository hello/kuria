#include "noveldaprotobuf.h"
#include "zmq_subscriber.h"
#include "proto_utils.h"
#include "novelda.pb.h"
#include <string.h>
#include "pb_decode.h"

bool NoveldaProtobuf::deserialize_protobuf(const uint8_t * protobuf_bytes, const size_t protobuf_size, NoveldaData & deserialized_data) {
    
    novelda_RadarFrame frame;
    
    
    double buf[1024]; //1024 range bins, should be enough
    
    BufferInfo_t info = {reinterpret_cast<uint8_t * >(buf),sizeof(buf),0};
    
    pb_istream_t istream = pb_istream_from_buffer(protobuf_bytes, protobuf_size);

    memset(&frame,0,sizeof(frame));

    frame.range_bins.funcs.decode = decode_repeated_doubles;
    frame.range_bins.arg = (void *)&info;
    
    if (!pb_decode(&istream,novelda_RadarFrame_fields,&frame)) {
        return false;
    }
    
    const size_t num_items_received = info.pos / sizeof(double);
    
    if (num_items_received == 0) {
        return false;
    }
    
    deserialized_data.frame_id = 0;
    deserialized_data.is_base_band = false;
    
    if (frame.has_frame_id) {
        deserialized_data.frame_id = frame.frame_id;
    }
    
    
    if (frame.has_base_band) {
        deserialized_data.is_base_band = frame.base_band;
    }
    
    deserialized_data.range_bins.reserve(num_items_received);
    
    for (int i = 0; i < num_items_received; i++) {
        deserialized_data.range_bins.push_back(buf[i]);
    }
    
    return true;
    
}
