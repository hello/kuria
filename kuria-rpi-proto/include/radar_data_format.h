#ifndef _RADAR_DATA_FORMAT_H_
#define _RADAR_DATA_FORMAT_H_

#include <stdint.h>

typedef struct {
    uint32_t content_id; // indicates if data is IQ or amplitude/phase
    uint32_t frame_counter;
    uint32_t num_of_bins; // number of bins in dataset
    int64_t timestamp; // 64 bit epoch
    float bin_length; // length in meters between each bin
    float sampling_frequency; // chip sampling frequency in Hz
    float carrier_frequency; // chip carrier frequency in Hz
    float range_offset; // first range bin start in meters;
    float* sig_i; // array of num_of_bins float values of the signal I-channel
    float* sig_q; // array of num_of_bins float values of the signal q-channel
    float* fdata; // array of alternating I and Q channel float values
}radar_frame_packet;
#endif

