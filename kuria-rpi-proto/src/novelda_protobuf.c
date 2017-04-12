#include "novelda_protobuf.h"
#include "novelda.pb.h"
#include "pb_encode.h"
#include <stdlib.h>
#include <stdio.h>

bool decode_repeated_doubles(pb_istream_t *stream, const pb_field_t *field, void **arg); 

#if (RADAR_DATA_TESTING == 0)
int32_t radar_data_encode (uint8_t** buf, radar_frame_packet_t* packet){

    static uint32_t frame_id_test = 0; 
    novelda_RadarFrame radar_frame;

    if (!packet){
        printf ("invalid data to encode\n");
        return -1;
    }

    // initialize to 0
    memset (&radar_frame, 0, sizeof (novelda_RadarFrame));

    // initialize frame id, sequential counter, incremented for every message
    radar_frame.has_frame_id = true;
#if 0
    radar_frame.frame_id = packet->frame_counter; // TODO verify this is populated by radar task 
#else
    radar_frame.frame_id = frame_id_test; // TODO verify this is populated by radar task 
    frame_id_test++;
#endif

    // Indicates if baseband data or raw
    radar_frame.has_base_band = true;
    radar_frame.base_band = packet->content_id;// TODO verify this is populated by radar task

    // set callbacks to encode the radar data
    radar_frame.range_bins.funcs.encode = _encode_range_bins;
    radar_frame.range_bins.arg = packet;

    size_t size;
    bool ret = pb_get_encoded_size (&size, novelda_RadarFrame_fields, &radar_frame);

    *buf = (uint8_t*) malloc (size);

    pb_ostream_t ostream = pb_ostream_from_buffer (*buf,size);

    if (!pb_encode (&ostream, novelda_RadarFrame_fields, &radar_frame)) {
        printf (" failed to encode radar data: %d\n", packet->frame_counter);
        return -1;
    }

    return ostream.bytes_written;

}

bool _encode_range_bins (pb_ostream_t* stream, const pb_field_t* field, void* const* arg) {

    radar_frame_packet_t* packet = (radar_frame_packet_t*) (*arg);

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

    uint32_t i;
    // encode the double repeated field for range bins
    for (i=0; i<packet->num_of_bins;i++) {
        // all data in this driver is float32_t. only protobuf is double
        double value;
        value = (double) packet->fdata[i];
        if (!pb_encode_fixed64 (stream, &value )) {
            return false;
        }
    }

    return true;
}
#endif
// TODO define this only for radar_subscriber
// #define RADAR_DATA_TESTING 1
#if RADAR_DATA_TESTING
#include "pb_decode.h"

typedef struct {
    uint8_t * buffer;
    size_t buf_size_bytes;
    size_t pos;
} BufferInfo_t;

int32_t radar_data_decode (uint8_t* protobuf_bytes, const size_t protobuf_size, radar_frame_packet_t* packet ) {


    double buf[1024] = {0};
    novelda_RadarFrame frame;
    BufferInfo_t info = {(uint8_t*)buf, sizeof(buf), 0};
    pb_istream_t istream = pb_istream_from_buffer (protobuf_bytes, protobuf_size);

    memset (&frame, 0, sizeof (frame) );

    frame.range_bins.funcs.decode = decode_repeated_doubles;
    frame.range_bins.arg = (void*) &info;

    if (!pb_decode (&istream, novelda_RadarFrame_fields, &frame) ) {
        printf ("failed to decode %s\n", PB_GET_ERROR (&istream));
        return -1;
    }

    const size_t num_items_received = info.pos / sizeof (double);

//    printf ("items received is %d\n", num_items_received);
    printf ("Frame id: %d\n", frame.frame_id);
    if (num_items_received == 0) {
        return -1;
    }

    packet->frame_counter = 0 ;
    packet->content_id = 0;

    if (frame.has_frame_id) {
        packet->frame_counter = frame.frame_id;
    }

    if (frame.has_base_band) {
        packet->content_id = frame.base_band;
    }
#if 0
    for (int i = 0; i < num_items_received/2; i++) {
        packet->fdata[2*i] = buf[2*i];
        packet->fdata[2*i+1] = buf[2*i+1];
    }
#endif
    return 0;
}

bool decode_repeated_doubles(pb_istream_t *stream, const pb_field_t *field, void **arg) {
    BufferInfo_t * info = (BufferInfo_t *) *arg;
    bool ret;

    while(stream->bytes_left > 0) {
        double * dbuf = (double *)(info->buffer + info->pos);

        if (info->pos + sizeof(double) > info->buf_size_bytes) {
            printf ("repeated double fail\n");
            return false;
        }


        ret = pb_decode_fixed64(stream, dbuf);
        if (!ret) {
            printf ("decode double failed\n");
            return ret;
        }
        info->pos += sizeof(double);
    }

    return true;

}

#endif
