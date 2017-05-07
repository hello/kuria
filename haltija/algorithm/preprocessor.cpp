#include "preprocessor.h"
#include "haltijamath.h"
#include "filters.h"


using namespace Eigen;
/*
Preprocessor::Preprocessor()
: _num_samples_per_frame(1) {
    
 
}
 */


PreprocessorPtr_t Preprocessor::createWithPostiveFreqBandpass(const int num_range_bins,const int num_frames_in_segment,const int num_frames_to_wait_between_segments) {
    
    Eigen::MatrixXcf Bplus(21,1);
    Bplus <<
    Complex_t(0.00500753,0.00000000),
    Complex_t(0.00569707,0.00413917),
    Complex_t(0.00385975,0.01187909),
    Complex_t(-0.00663818,0.02043021),
    Complex_t(-0.02716560,0.01973696),
    Complex_t(-0.04780158,0.00000000),
    Complex_t(-0.05076078,-0.03687987),
    Complex_t(-0.02372087,-0.07300535),
    Complex_t(0.02726047,-0.08389911),
    Complex_t(0.07743443,-0.05625940),
    Complex_t(0.09832260,-0.00000000),
    Complex_t(0.07743443,0.05625940),
    Complex_t(0.02726047,0.08389911),
    Complex_t(-0.02372087,0.07300535),
    Complex_t(-0.05076078,0.03687987),
    Complex_t(-0.04780158,0.00000000),
    Complex_t(-0.02716560,-0.01973696),
    Complex_t(-0.00663818,-0.02043021),
    Complex_t(0.00385975,-0.01187909),
    Complex_t(0.00569707,-0.00413917),
    Complex_t(0.00500753,-0.00000000);
    
    
    //reverse
    MatrixXcf B(Bplus.rows(),1);
    
    for (int i = 0; i < Bplus.rows(); i++) {
        B(i,0) = Bplus(Bplus.rows() - i - 1,0);
    }
    
    return PreprocessorPtr_t(new Preprocessor(B,num_range_bins,num_frames_in_segment,num_frames_to_wait_between_segments));

    
}

PreprocessorPtr_t Preprocessor::createWithNegativeFreqBandpass(const int num_range_bins,const int num_frames_in_segment,const int num_frames_to_wait_between_segments) {
    
    Eigen::MatrixXcf Bminus(21,1);
    Bminus <<
    Complex_t(0.00500753,0.00000000),
    Complex_t(0.00569707,-0.00413917),
    Complex_t(0.00385975,-0.01187909),
    Complex_t(-0.00663818,-0.02043021),
    Complex_t(-0.02716560,-0.01973696),
    Complex_t(-0.04780158,-0.00000000),
    Complex_t(-0.05076078,0.03687987),
    Complex_t(-0.02372087,0.07300535),
    Complex_t(0.02726047,0.08389911),
    Complex_t(0.07743443,0.05625940),
    Complex_t(0.09832260,0.00000000),
    Complex_t(0.07743443,-0.05625940),
    Complex_t(0.02726047,-0.08389911),
    Complex_t(-0.02372087,-0.07300535),
    Complex_t(-0.05076078,-0.03687987),
    Complex_t(-0.04780158,-0.00000000),
    Complex_t(-0.02716560,0.01973696),
    Complex_t(-0.00663818,0.02043021),
    Complex_t(0.00385975,0.01187909),
    Complex_t(0.00569707,0.00413917),
    Complex_t(0.00500753,0.00000000);
    
    //reverse
    MatrixXcf B(Bminus.rows(),1);
    
    for (int i = 0; i < Bminus.rows(); i++) {
        B(i,0) = Bminus(Bminus.rows() - i - 1,0);
    }
    
    return PreprocessorPtr_t(new Preprocessor(B,num_range_bins,num_frames_in_segment,num_frames_to_wait_between_segments));

}


Preprocessor::Preprocessor(const MatrixXcf & high_pass_filter_coeff,const int num_range_bins,const int num_frames_in_segment,const int num_frames_to_wait_between_segments)
:_num_frames_in_segment(num_frames_in_segment)
,_num_frames_to_wait_between_segments(num_frames_to_wait_between_segments)
,_input_idx(0)
,_output_idx(0)
,_is_input_full(false)
,_idx_sample(0)
{
    
    _high_pass_filter_coeff = high_pass_filter_coeff;
    _input_circular_buffer = MatrixXcf::Zero(_high_pass_filter_coeff.rows(),num_range_bins);
    _output_circular_buffer = MatrixXcf::Zero(_num_frames_in_segment,num_range_bins);
    _num_frames_to_wait_between_segments = num_frames_in_segment;
    _num_frames_in_segment = num_frames_in_segment;
    
}

Preprocessor::~Preprocessor() {
    
}
 
void Preprocessor::reset() {
    _input_idx = 0;
    _output_idx = 0;
    _idx_sample = 0;
}


uint32_t Preprocessor::add_frame(const BasebandDataFrame_t & input, MatrixXcf & filtered_frame, MatrixXcf & segment)  {
    uint32_t flags = PREPROCESSOR_FLAGS_NOT_READY;
    MatrixXcf input_row(1,input.data.size());
    
    //copy
    for (int i = 0; i < input.data.size(); i++) {
        _input_circular_buffer(_input_idx,i) = input.data[i];
    }
    
    if (++_input_idx >= _input_circular_buffer.rows()) {
        _input_idx = 0;
        _is_input_full = true;
    }
    
    if (_is_input_full) {
        flags |= PREPROCESSOR_FLAGS_FRAME_READY;
    }
    
    //circular shift FIR filter coefficients
    const MatrixXcf B = circular_shift_columns(_high_pass_filter_coeff,_input_idx);
    
    filtered_frame = fir_filter_columns(B, _input_circular_buffer);
    
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



