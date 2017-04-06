#ifndef __NOVELDA_PROTOBUF_H__
#define __NOVELDA_PROTOBUF_H__


#include "radar_data_format.h"

int32_t radar_data_encode (novelda_RadarFrame* radar_frame, radar_frame_packet_t* radar_packet);

#endif
