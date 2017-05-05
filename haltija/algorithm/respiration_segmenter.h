#ifndef _RESPIRATION_SEGMENTER_H_
#define _RESPIRATION_SEGMENTER_H_

#include <Eigen/Core>

#include "haltija_types.h"
#include <array>

enum {
    exhaled = 0,
    inhaling,
    inhaled,
    exhaling,
    NUM_RESPIRATION_STATES
} ERespirationState_t;

typedef std::array<float,NUM_RESPIRATION_STATES> RespirationStateFloatArray_t;

class RespirationPrediction {
public:
    RespirationPrediction();
    RespirationStateFloatArray_t respiration_probs;    
};

class RespirationSegmenter {
public:
    
    RespirationSegmenter();
    
    void set_segment(const Eigen::MatrixXcf segment, const Eigen::MatrixXcf buffered_live_signal, bool is_respiration);
    
    RespirationPrediction predict_respiration_state(const Eigen::MatrixXcf & transformed_frame, const float sample_rate_hz);
private:
    void bayes_update(const Eigen::MatrixXcf & transformed_frame);
    void hmm_segmenter(const Eigen::MatrixXf & x,const Eigen::MatrixXcf & orig, Complex_t  * resipration_clusters) const;
    Complex_t _respiration_clusters[NUM_RESPIRATION_STATES];
    float _variance;
    Eigen::MatrixXf _state;
    Eigen::MatrixXf _state_transition_matrix;
};

#endif //_RESPIRATION_SEGMENTER_H_
