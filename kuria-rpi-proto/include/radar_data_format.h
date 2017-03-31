#ifndef _RADAR_DATA_FORMAT_H_
#define _RADAR_DATA_FORMAT_H_

#include <stdint.h>
#include "kuria_utils.h"


typedef struct {
    uint32_t content_id; // indicates if data is IQ or amplitude/phase
    uint32_t frame_counter;
    uint32_t num_of_bins; // number of bins in dataset
    int64_t timestamp; // 64 bit epoch
    float32_t bin_length; // length in meters between each bin
    float32_t sampling_frequency; // chip sampling frequency in Hz
    float32_t carrier_frequency; // chip carrier frequency in Hz
    float32_t range_offset; // first range bin start in meters;
    float32_t* sig_i; // array of num_of_bins float values of the signal I-channel
    float32_t* sig_q; // array of num_of_bins float values of the signal q-channel
    float32_t* fdata; // array of alternating I and Q channel float values
}radar_frame_packet_t;
#endif

