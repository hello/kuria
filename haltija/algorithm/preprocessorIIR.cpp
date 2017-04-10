#include "preprocessorIIR.h"
#include <iostream>
using namespace Eigen;

PreprocessorIIR::PreprocessorIIR(RealCoeffComplexDataFilt_t * phpf, RealCoeffComplexDataFilt_t * plpf, const int num_range_bins,const int num_frames_in_segment,const int num_frames_to_wait_between_segments, float scale)
:_num_frames_in_segment(num_frames_in_segment)
,_num_frames_to_wait_between_segments(num_frames_to_wait_between_segments)
,_output_idx(0)
,_idx_sample(0)
,_phpf(phpf)
,_plpf(plpf)
,_scale(scale)
{

    _raw_segment = MatrixXcf::Zero(_num_frames_in_segment,num_range_bins);

}

PreprocessorIIR::~PreprocessorIIR() {
    
    if (_phpf)
        delete _phpf;

    if (_plpf)
        delete _plpf;
}

void PreprocessorIIR::reset() {
    
    if (_phpf)
        _phpf->reset();
    
    if (_plpf)
        _plpf->reset();
}

PreprocessorPtr_t PreprocessorIIR::createWithDefaultHighpassFilter(const int num_range_bins,const int num_frames_in_segment,const int num_frames_to_wait_between_segments, float scale) {
    
    
    MatrixXf Bhpf(2,1);
    MatrixXf Ahpf(2,1);
    
    

    Bhpf <<  0.99601935, -0.99601935;
    Ahpf <<  1.       , -0.9920387;

    
    /*
    //In [78]: B,A = sig.iirdesign(wp=0.2/10.0,ws=0.02 / 10.0,gpass=2.0,gstop = 10.0,ftype='butter')
    Bhpf <<  0.97652981, -0.97652981;
    Ahpf <<  1.        , -0.95305962;
*/
    
    /*
    //In [81]: B,A = sig.iirdesign(wp=0.1/10.0,ws=0.01 / 10.0,gpass=2.0,gstop = 10.0,ftype='butter')
    Bhpf <<  0.98812845, -0.98812845;
    Ahpf <<  1.        , -0.97625691;
     */
    
    auto phpf = new IIRFilter<Eigen::MatrixXf, Eigen::MatrixXcf>(Bhpf,Ahpf,num_range_bins);

    return PreprocessorPtr_t(new PreprocessorIIR(phpf,NULL,num_range_bins,num_frames_in_segment,num_frames_to_wait_between_segments,scale));
  
}

PreprocessorPtr_t PreprocessorIIR::createWithDefaultHighpassFilterAndLowpass(const int num_range_bins,const int num_frames_in_segment,const int num_frames_to_wait_between_segments,float scale) {

    
    MatrixXf Bhpf(2,1);
    MatrixXf Ahpf(2,1);
    
    Bhpf <<  0.99601935, -0.99601935;
    Ahpf <<  1.       , -0.9920387;
    MatrixXf Blpf(2,1);
    MatrixXf Alpf(2,1);
    
    //B,A = sig.iirdesign(wp=0.2/10.0,ws=4.0 / 10.0,gpass=2.0,gstop = 20.0,ftype='butter')
    Blpf <<   0.03946985,  0.03946985;
    Alpf << 1.       , -0.9210603;
    
    auto phpf = new IIRFilter<Eigen::MatrixXf, Eigen::MatrixXcf>(Bhpf,Ahpf,num_range_bins);
    auto plpf = new IIRFilter<Eigen::MatrixXf, Eigen::MatrixXcf>(Blpf,Alpf,num_range_bins);
    
    return PreprocessorPtr_t(new PreprocessorIIR(phpf,plpf,num_range_bins,num_frames_in_segment,num_frames_to_wait_between_segments,scale));

    
}

uint32_t PreprocessorIIR::add_frame(const BasebandDataFrame_t &input, Eigen::MatrixXcf &filtered_frame, Eigen::MatrixXcf &segment) {
    
    uint32_t flags = PREPROCESSOR_FLAGS_NOT_READY;
    MatrixXcf raw(1,input.data.size());
    
    //copy
    for (int i = 0; i < input.data.size(); i++) {
        raw(0,i) = input.data[i] * _scale;
    }

    if (_idx_sample == 0) {
        _phpf->reset_to_output(raw, MatrixXcf::Zero(raw.rows(),raw.cols())); // force hpf
    }
    
    //highpass filter
    filtered_frame = _phpf->filter(raw);
    
    flags |= PREPROCESSOR_FLAGS_FRAME_READY;

    
     //insert raw and optionall lowpass filtered segment
    _raw_segment.row(_output_idx) = filtered_frame;
    
    /*
    if (_plpf) {
        _lpf_segment.row(_output_idx) = _plpf->filter(raw);
    }
     */

    
    if (++_output_idx >= _raw_segment.rows()) {
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
    segment = _raw_segment;
    
    //circular shift
    segment = circular_shift_columns(segment, -(_idx_sample % _num_frames_in_segment));

    /*
    MatrixXf Blpf(2,1);
    MatrixXf Alpf(2,1);

    Blpf <<   0.03946985,  0.03946985;
    Alpf << 1.       , -0.9210603;

    IIRFilter<MatrixXf,MatrixXcf> lpf(Blpf,Alpf,segment.cols());
    segment = lpf.filtfilt(segment);
    */
    
    return flags;

}
