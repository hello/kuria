#ifndef _PREPROCESSOR_H_
#define _PREPROCESSOR_H_

#include "preprocessor_interface.h"
#include "circbuf.h"
#include <vector>
#include <stdint.h>



class Preprocessor : public PreprocessorInterface {
    
private:
    //private constructor so this can only be created with create methods
    Preprocessor(const Eigen::MatrixXcf & filter_coefficeints,const int num_range_bins,const int num_frames_in_segment,const int num_frames_to_wait_between_segments);
    
public:
    ~Preprocessor();
    
    void reset();
    
    //return flags
    uint32_t add_frame(const BasebandDataFrame_t & input, Eigen::MatrixXcf & filtered_frame, Eigen::MatrixXcf & segment);
    
    static PreprocessorPtr_t createWithDefaultHighpassFilter(const int num_range_bins,const int num_frames_in_segment,const int num_frames_to_wait_between_segments);

    static PreprocessorPtr_t createWithPostiveFreqBandpass(const int num_range_bins,const int num_frames_in_segment,const int num_frames_to_wait_between_segments);
    
    static PreprocessorPtr_t createWithNegativeFreqBandpass(const int num_range_bins,const int num_frames_in_segment,const int num_frames_to_wait_between_segments);

    
private:
    
    int _num_frames_in_segment;
    int _num_frames_to_wait_between_segments;
    
    Eigen::MatrixXcf _high_pass_filter_coeff;
    Eigen::MatrixXcf _input_circular_buffer;
    Eigen::MatrixXcf _output_circular_buffer;

    int _input_idx;
    int _output_idx;
    
    bool _is_input_full;
    
    size_t _idx_sample;

};


#endif //_PREPROCESSOR_H_
