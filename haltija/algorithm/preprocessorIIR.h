#ifndef _PREPROCESSORIIR_H_
#define _PREPROCESSORIIR_H_

#include "preprocessor_interface.h"
#include "filters.h"

class PreprocessorIIR : public PreprocessorInterface {
private:
    PreprocessorIIR(const Eigen::MatrixXf & B,const Eigen::MatrixXf & A, const int num_range_bins,const int num_frames_in_segment,const int num_frames_to_wait_between_segments);

    
    
    
public:
    static PreprocessorPtr_t createWithDefaultHighpassFilter(const int num_range_bins,const int num_frames_in_segment,const int num_frames_to_wait_between_segments);
    
    ~PreprocessorIIR();
    
    void reset();
    
    uint32_t add_frame(const BasebandDataFrame_t & input, Eigen::MatrixXcf & filtered_frame, Eigen::MatrixXcf & segment);
    
private:
    
    int _num_frames_in_segment;
    int _num_frames_to_wait_between_segments;
    
    Eigen::MatrixXcf _output_circular_buffer;
    
    IIRFilter<Eigen::MatrixXf, Eigen::MatrixXcf> * _phpf;
    
    int _output_idx;
        
    size_t _idx_sample;
    
};

#endif
