#ifndef _PREPROCESSORIIR_H_
#define _PREPROCESSORIIR_H_

#include "preprocessor_interface.h"
#include "filters.h"

class PreprocessorIIR : public PreprocessorInterface {
private:
    typedef IIRFilter<Eigen::MatrixXf, Eigen::MatrixXcf> RealCoeffComplexDataFilt_t;
    
    PreprocessorIIR(RealCoeffComplexDataFilt_t * hpf,RealCoeffComplexDataFilt_t * lpf, const int num_range_bins,const int num_frames_in_segment,const int num_frames_to_wait_between_segments, float scale);

    
public:
    static PreprocessorPtr_t createWithDefaultHighpassFilter(const int num_range_bins,const int num_frames_in_segment,const int num_frames_to_wait_between_segments,float scale);
    
    static PreprocessorPtr_t createWithDefaultHighpassFilterAndLowpass(const int num_range_bins,const int num_frames_in_segment,const int num_frames_to_wait_between_segments,float scale);

    static PreprocessorPtr_t createWithAggressiveHighpassFilterAndLowpass(const int num_range_bins,const int num_frames_in_segment,const int num_frames_to_wait_between_segments,float scale);

    
    ~PreprocessorIIR();
    
    void reset();
    
    uint32_t add_frame(const BasebandDataFrame_t & input, Eigen::MatrixXcf & filtered_frame, Eigen::MatrixXcf & segment);
    
private:
    
    int _num_frames_in_segment;
    int _num_frames_to_wait_between_segments;
    
    Eigen::MatrixXcf _raw_segment;

    int _output_idx;
    size_t _idx_sample;
    
    RealCoeffComplexDataFilt_t * _phpf;
    RealCoeffComplexDataFilt_t * _plpf;
    
    float _scale;

};

#endif
