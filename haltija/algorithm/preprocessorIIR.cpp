#include "preprocessorIIR.h"

using namespace Eigen;

PreprocessorIIR::PreprocessorIIR(const Eigen::MatrixXf & B,const Eigen::MatrixXf & A, const int num_range_bins,const int num_frames_in_segment,const int num_frames_to_wait_between_segments)
:_num_frames_in_segment(num_frames_in_segment)
,_num_frames_to_wait_between_segments(num_frames_to_wait_between_segments)
,_output_idx(0)
,_idx_sample(0)
{

    _output_circular_buffer = MatrixXcf::Zero(_num_frames_in_segment,num_range_bins);
    _num_frames_to_wait_between_segments = num_frames_in_segment;
    _num_frames_in_segment = num_frames_in_segment;
    _phpf = new IIRFilter<Eigen::MatrixXf, Eigen::MatrixXcf>(B,A,num_range_bins);
}

PreprocessorIIR::~PreprocessorIIR() {
    delete _phpf;
}

void PreprocessorIIR::reset() {
    _phpf->reset();
}

PreprocessorPtr_t PreprocessorIIR::createWithDefaultHighpassFilter(const int num_range_bins,const int num_frames_in_segment,const int num_frames_to_wait_between_segments) {
    
    
    MatrixXf B(5,1);
    MatrixXf A(5,1);
    
    B <<  0.85284624, -1.70569249,  0.85284624;
    A << 1.        , -1.68391975,  0.72746523;
    
    
    return PreprocessorPtr_t(new PreprocessorIIR(B,A,num_range_bins,num_frames_in_segment,num_frames_to_wait_between_segments));
  
}



uint32_t PreprocessorIIR::add_frame(const BasebandDataFrame_t &input, Eigen::MatrixXcf &filtered_frame, Eigen::MatrixXcf &segment) {

    
    uint32_t flags = PREPROCESSOR_FLAGS_NOT_READY;
    MatrixXcf input_row(1,input.data.size());
    
    //copy
    for (int i = 0; i < input.data.size(); i++) {
        input_row(0,i) = input.data[i];
    }

    //highpass filter
    filtered_frame = _phpf->filter(input_row);
    flags |= PREPROCESSOR_FLAGS_FRAME_READY;

    
     //insert
    _output_circular_buffer.row(_output_idx) = filtered_frame;
    
    if (++_output_idx >= _output_circular_buffer.rows()) {
        _output_idx = 0;
    }
    
    _idx_sample++;
    
    // we have enough points for a segment ?
    if (_idx_sample < _num_frames_in_segment) {
        return flags;
    }
    
    //we've hit a segment period?
    if (_idx_sample % _num_frames_to_wait_between_segments != 0) {
        return flags;
    }
    
    flags |= PREPROCESSOR_FLAGS_SEGMENT_READY;
    
    //copy
    segment = _output_circular_buffer;
    
    return flags;

}
