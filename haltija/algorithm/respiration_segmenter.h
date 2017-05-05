#ifndef _RESPIRATION_SEGMENTER_H_
#define _RESPIRATION_SEGMENTER_H_

#include <Eigen/Core>

#include "haltija_types.h"
#include <array>

enum {
    exhaled = 0,
    exhaling,
    inhaled,
    inhaling,
    NUM_RESPIRATION_STATES
} ERespirationState_t;

typedef std::array<float,NUM_RESPIRATION_STATES> RespirationStateFloatArray_t;

class RespirationPrediction {
public:
    RespirationPrediction();
    RespirationStateFloatArray_t respiration_probs;
    RespirationStateFloatArray_t respiration_probs_deriv;
    float inhaleexhale; //positive is exhale;
    
};

class RespirationSegmenter {
public:
    
    void set_segment(const Eigen::MatrixXcf segment, const Eigen::MatrixXcf buffered_live_signal);
    
    RespirationPrediction predict_respiration_state(const Eigen::MatrixXcf & transformed_frame, const float sample_rate_hz);
private:
    RespirationStateFloatArray_t eval_pdfs(const Eigen::MatrixXcf & transformed_frame);
    Complex_t _respiration_clusters[NUM_RESPIRATION_STATES];
    float _variance;
    RespirationPrediction _prev_prediction;
};

#endif //_RESPIRATION_SEGMENTER_H_
