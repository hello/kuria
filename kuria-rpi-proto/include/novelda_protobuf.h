#ifndef __NOVELDA_PROTOBUF_H__
#define __NOVELDA_PROTOBUF_H__


#include "novelda.pb.h"
#include "radar_data_format.h"

int32_t radar_data_encode (uint8_t** buf, radar_frame_packet_t* radar_packet);
bool _encode_range_bins (pb_ostream_t* stream, const pb_field_t* field, void* const* arg); 

#endif
