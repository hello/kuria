#ifndef __DISPATCH_RADAR_FRAME_H__
#define __DISPATCH_RADAR_FRAME_H__

#include "radar_data_format.h"

int32_t dispatcher_init (void); 
int32_t dispatch_radar_frame (radar_frame_packet_t* packet); 
int32_t dispatcher_close (void);

#endif
