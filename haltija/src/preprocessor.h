#ifndef _PREPROCESSOR_H_
#define _PREPROCESSOR_H_

#include "haltija_types.h"
#include "circbuf.h"
#include <vector>
#include <Eigen/Core>
#include <memory>

class Preprocessor;
typedef std::shared_ptr<Preprocessor> PreprocessorPtr_t;

class Preprocessor {
    
private:
    //private constructor so this can only be created with create methods
    Preprocessor(const Eigen::MatrixXf & filter_coefficeints,const int num_range_bins,const int num_frames_in_segment,const int num_frames_to_wait_between_segments);
    
public:
    ~Preprocessor();
    
    void reset_counters();
    
    //return true when a segment is ready
    bool add_frame(const BasebandDataFrame_t & input, Eigen::MatrixXcf & filtered_frame, Eigen::MatrixXcf & segment);
    
    //create with this
    static PreprocessorPtr_t createWithDefaultHighpassFilter(const int num_range_bins,const int num_frames_in_segment,const int num_frames_to_wait_between_segments);
    
private:
    
    int _num_frames_in_segment;
    int _num_frames_to_wait_between_segments;
    
    Eigen::MatrixXf _high_pass_filter_coeff;
    Eigen::MatrixXcf _input_circular_buffer;
    Eigen::MatrixXcf _output_circular_buffer;

    int _input_idx;
    int _output_idx;
    
    size_t _idx_sample;

};


#endif //_PREPROCESSOR_H_
