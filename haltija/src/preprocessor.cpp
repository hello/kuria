#include "preprocessor.h"
#include "haltijamath.h"
#include "filters.h"


using namespace Eigen;
/*
Preprocessor::Preprocessor()
: _num_samples_per_frame(1) {
    
    _B_high_pass.resize(101,1);
    
    _B_high_pass << -5.09552735e-04,  -5.19377203e-04,  -5.27693802e-04,
    -5.32186651e-04,  -5.29028364e-04,  -5.13008822e-04,
    -4.77777630e-04,  -4.16194545e-04,  -3.20776300e-04,
    -1.84222709e-04,  -6.21914388e-18,   2.37044837e-04,
    5.30067912e-04,   8.79641608e-04,   1.28322862e-03,
    1.73474745e-03,   2.22425768e-03,   2.73778937e-03,
    3.25733617e-03,   3.76102551e-03,   4.22347225e-03,
    4.61631472e-03,   4.90892434e-03,   5.06927203e-03,
    5.06492751e-03,   4.86416063e-03,   4.43710843e-03,
    3.75696714e-03,   2.80116515e-03,   1.55247220e-03,
    -2.79349436e-17,  -1.85994794e-03,  -4.02321562e-03,
    -6.47735573e-03,  -9.20143834e-03,  -1.21661247e-02,
    -1.53340142e-02,  -1.86602635e-02,  -2.20934658e-02,
    -2.55767708e-02,  -2.90492135e-02,  -3.24472146e-02,
    -3.57062081e-02,  -3.87623433e-02,  -4.15542096e-02,
    -4.40245269e-02,  -4.61217462e-02,  -4.78015080e-02,
    -4.90279088e-02,  -4.97745343e-02,   9.50479234e-01,
    -4.97745343e-02,  -4.90279088e-02,  -4.78015080e-02,
    -4.61217462e-02,  -4.40245269e-02,  -4.15542096e-02,
    -3.87623433e-02,  -3.57062081e-02,  -3.24472146e-02,
    -2.90492135e-02,  -2.55767708e-02,  -2.20934658e-02,
    -1.86602635e-02,  -1.53340142e-02,  -1.21661247e-02,
    -9.20143834e-03,  -6.47735573e-03,  -4.02321562e-03,
    -1.85994794e-03,  -2.79349436e-17,   1.55247220e-03,
    2.80116515e-03,   3.75696714e-03,   4.43710843e-03,
    4.86416063e-03,   5.06492751e-03,   5.06927203e-03,
    4.90892434e-03,   4.61631472e-03,   4.22347225e-03,
    3.76102551e-03,   3.25733617e-03,   2.73778937e-03,
    2.22425768e-03,   1.73474745e-03,   1.28322862e-03,
    8.79641608e-04,   5.30067912e-04,   2.37044837e-04,
    -6.21914388e-18,  -1.84222709e-04,  -3.20776300e-04,
    -4.16194545e-04,  -4.77777630e-04,  -5.13008822e-04,
    -5.29028364e-04,  -5.32186651e-04,  -5.27693802e-04,
    -5.19377203e-04,  -5.09552735e-04;
    
}
 */
Preprocessor::Preprocessor(const MatrixXf & high_pass_filter_coeff)
:_num_frames_in_segment(0)
,_num_frames_to_wait_between_segments(0)
,_input_idx(0)
,_output_idx(0)
,_idx_sample(0){
    _high_pass_filter_coeff = high_pass_filter_coeff;
}

Preprocessor::~Preprocessor() {
    
}
 
void Preprocessor::initialize(const int num_range_bins,const int num_frames_in_segment,const int num_frames_to_wait_between_segments) {
 
    _input_circular_buffer = MatrixXcf::Zero(_high_pass_filter_coeff.rows(),num_range_bins);
    _output_circular_buffer = MatrixXcf::Zero(_num_frames_in_segment,num_range_bins);
    _num_frames_to_wait_between_segments = num_frames_in_segment;
    _num_frames_in_segment = num_frames_in_segment;
    
    _input_idx = 0;
    _output_idx = 0;
    _idx_sample = 0;
    
}


bool Preprocessor::add_frame(const BasebandDataFrame_t & input, MatrixXcf & filtered_frame, MatrixXcf & segment)  {

    MatrixXcf input_row(1,input.data.size());
    
    //copy
    for (int i = 0; i < input.data.size(); i++) {
        _input_circular_buffer(_input_idx,i) = input.data[i];
    }
    
    if (++_input_idx >= _input_circular_buffer.size()) {
        _input_idx = 0;
    }
    
    //circular shift FIR filter coefficients
    const MatrixXf B = circular_shift_columns(_high_pass_filter_coeff,_input_idx);
    
    filtered_frame = fir_filter_columns(B, _input_circular_buffer);
    
    //insert
    _output_circular_buffer.row(_output_idx) = filtered_frame;
    
    if (++_output_idx >= _output_circular_buffer.size()) {
        _output_idx = 0;
    }
    
    _idx_sample++;
    
    // we have enough points for a segment ?
    if (_idx_sample < _num_frames_in_segment) {
        return false;
    }
    
    //we've hit a segment period?
    if (_idx_sample % _num_frames_to_wait_between_segments != 0) {
        return false;
    }
    
    //copy
    segment = _output_circular_buffer;
    
    return true;
    
}



