#include "novelda_protobuf.h"
#include "novelda.pb.h"
#include "pb_encode.h"

#include <stdio.h>

static bool _encode_range_bins (pb_ostream_t* stream, const pb_field_t* field, void* const* arg); 



int32_t radar_data_encode (novelda_RadarFrame* radar_frame, radar_frame_packet_t* packet){

    if (!radar_frame || !packet){
        printf ("invalid data to encode\n");
        return -1;
    }

    // should this be double or uint8_t TODO
    uint8_t buf[1024];// randomly chosen to match kuria decode function, TODO change later if needed

    // TODO not sure how to compute size of ostream buffer
    //    size_t protobuf_size = sizeof (double) * packet->num_of_bins + sizeof (bool) + sizeof (

    // open output stream 
    pb_ostream_t ostream = pb_ostream_from_buffer (buf,sizeof (buf));

    // initialize to 0
    memset (radar_frame, 0, sizeof (novelda_RadarFrame));

    // initialize frame id, sequential counter, incremented for every message
    radar_frame->has_frame_id = true;
    radar_frame->frame_id = packet->frame_counter; // TODO verify this is populated by radar task 

    // Indicates if baseband data or raw
    radar_frame->has_base_band = true;
    radar_frame->base_band = packet->content_id;// TODO verify this is populated by radar task

    // set callbacks to encode the radar data
    radar_frame->range_bins.funcs.encode = _encode_range_bins;
    radar_frame->range_bins.arg = packet;

    if (!pb_encode (&ostream, novelda_RadarFrame_fields, &radar_frame)) {
        printf (" failed to encode radar data: %d\n", packet->frame_counter);
        return -1;
    }

    return 0;

}

static bool _encode_range_bins (pb_ostream_t* stream, const pb_field_t* field, void* const* arg) {

    radar_frame_packet_t* packet = (radar_frame_packet_t*) arg;

    // encode wire type and tag of the field
    if (!pb_encode_tag_for_field (stream, field) ) {
        printf ("pb_encode_tag_for_field failed\n");
        return false;
    }

    // encode payload size
    if (!pb_encode_varint (stream, packet->num_of_bins) ) {
        printf ("pb encode payload size failed\n");
        return false;
    }

    // encode the double repeated field for range bins
    for (uint32_t i=0; i<packet->num_of_bins;i++) {
        // TODO Need to make sure data is double, or should proto be float
        double value;
        value = (double) packet->fdata[i];
        if (!pb_encode_fixed64 (stream, &value )) {
            return false;
        }
    }

    return true;
}
