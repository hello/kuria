#include "respiration_segmenter.h"
#include "respiration_classifier.h"
#include <float.h>
#include "haltijamath.h"
#include "debug_publisher.h"
#include "filters.h"
#include "haltijamath.h"
#include "hmm/HmmHelpers.h"
#include "hmm/AllModels.h"


using Eigen::MatrixXcf;
using Eigen::MatrixXf;

RespirationPrediction::RespirationPrediction() {
    memset(respiration_probs.data(),0,respiration_probs.size()*sizeof(float));
    memset(respiration_probs_deriv.data(),0,respiration_probs.size()*sizeof(float));
    inhaleexhale = 0.0f;
}

static void hmm_segmenter(const MatrixXf & x,const Eigen::MatrixXcf & orig, Complex_t  * resipration_clusters) {
    
    
    float max = -1e10;
    float min  = 1e10;
    for (int i = 0; i < x.rows(); i++) {
        if (x(i,0) > max) {
            max = x(i,0);
        }
        
        if (x(i,0) < min) {
            min = x(i,0);
        }
    }
    
    float range = max - min;
    
    max -= 0.05 * range;
    min += 0.05 * range;
    
    HmmDataVec_t obs1;
    obs1.reserve(x.rows());
    
    for (int t = 0; t < x.rows(); t++) {
        obs1.push_back(x(t,0));
    }
    
    HmmDataMatrix_t meas = {obs1};
    
    const HmmDataVec_t pi = {1.0,1.0,1.0,1.0};
    const float extrema_std_dev = range * 0.3;
    const float transition_std_dev = 0.2 * range;
    const float midpoint = 0.0;
    //const int32_t obsnum,const float mean, const float stddev, const float weight
    ModelVec_t models_up = {
        HmmPdfInterfaceSharedPtr_t(new OneDimensionalGaussianModel(0,max,extrema_std_dev,1.0)),
        HmmPdfInterfaceSharedPtr_t(new OneDimensionalGaussianModel(0,midpoint,transition_std_dev,1.0)),
        HmmPdfInterfaceSharedPtr_t(new OneDimensionalGaussianModel(0,min,extrema_std_dev,1.0)),
        HmmPdfInterfaceSharedPtr_t(new OneDimensionalGaussianModel(0,midpoint,transition_std_dev,1.0))
    };
    
    ModelVec_t models_down = {
        HmmPdfInterfaceSharedPtr_t(new OneDimensionalGaussianModel(0,min,extrema_std_dev,1.0)),
        HmmPdfInterfaceSharedPtr_t(new OneDimensionalGaussianModel(0,midpoint,transition_std_dev,1.0)),
        HmmPdfInterfaceSharedPtr_t(new OneDimensionalGaussianModel(0,max,extrema_std_dev,1.0)),
        HmmPdfInterfaceSharedPtr_t(new OneDimensionalGaussianModel(0,midpoint,transition_std_dev,1.0))
    };
    
    
    HmmDataMatrix_t log_bmap_up;
    for (ModelVec_t::const_iterator it = models_up.begin(); it != models_up.end(); it++) {
        log_bmap_up.push_back((*it).get()->getLogOfPdf(meas));
    }
    
    HmmDataMatrix_t log_bmap_down;
    for (ModelVec_t::const_iterator it = models_down.begin(); it != models_down.end(); it++) {
        log_bmap_down.push_back((*it).get()->getLogOfPdf(meas));
    }
    
    HmmDataMatrix_t A;
    A.reserve(NUM_RESPIRATION_STATES);
    
    /*
     1 + r + r^2 + r^3 .... r^inf = 1 / (1 - r)
     
     1 / (1 - r) = x
     1 - r = 1/x
     
     r = 1 - 1/x
     
     */
    
    const float long_term = 20.0f;
    const float short_term = 1.0f;
    
    const float a1 = 1.0f - 1.0f / long_term;
    const float b1 = 1.0f - a1;
    
    const float a2 = 1.0f - 1.0f / short_term;
    const float b2 = 1.0 - a2;
    
    
    A.push_back({a1,b1,0,0});  //state 0 is exhaled
    A.push_back({0,a1,b1,0});  //state 1 is inhaling
    A.push_back({0,0,a2,b2});  //state 2 is inhaled
    A.push_back({b1,0,0,a1});  //state 3 is exhaling
    
    //AlphaBetaResult_t alphabeta = HmmHelpers::getAlphaAndBeta(meas.size(),pi, logBmap, A, A.size());
    UIntSet_t allowed_final_states = {0,1,2,3};
    ViterbiDecodeResult_t result_up = HmmHelpers::decodeWithoutLabels(A, log_bmap_up, pi, A.size(), obs1.size(),allowed_final_states);
    ViterbiDecodeResult_t result_down = HmmHelpers::decodeWithoutLabels(A, log_bmap_down, pi, A.size(), obs1.size(),allowed_final_states);
    
    
    
    auto path_up = result_up.getPath();
    auto path_down = result_down.getPath();
    
    
    float cost_up = result_up.getCost();
    float cost_down = result_down.getCost();
    
    ViterbiDecodeResult_t best_result = result_down;
    
    if (cost_up > cost_down) {
        best_result = result_up;
    }
    
    
    /* debug debug debug  */
    {
        Eigen::MatrixXf temp(best_result.getPath().size(),1);
        
        for (int i = 0; i < best_result.getPath().size(); i++) {
            temp(i,0) = best_result.getPath()[i];
        }
        
        debug_save("path",temp);
        debug_save("meas",x);
    }
    
    Eigen::MatrixXcf accumulator = Eigen::MatrixXcf::Zero(NUM_RESPIRATION_STATES, 1);
    
    
    int t = 0;
    int counts[NUM_RESPIRATION_STATES] = {0,0,0,0};
    for (auto it = best_result.getPath().begin(); it != best_result.getPath().end(); it++) {
        accumulator(*it,0) += orig(t++,0);
        counts[*it]++;
    }
    
    for (int i = 0; i < NUM_RESPIRATION_STATES; i++) {
        if (counts[i] == 0) {
            continue;
        }
        
        accumulator(i,0) /= Complex_t(counts[i],0);
    }
    
    
    for (int i = 0; i < NUM_RESPIRATION_STATES; i++) {
        resipration_clusters[i] = accumulator(i,0);
    }
    
}




void RespirationSegmenter::set_segment(const Eigen::MatrixXcf segment, const Eigen::MatrixXcf live_signal) {
    
    //get real bandpassed signal from complex signal
    const Eigen::MatrixXf projected_real_filtered_signal = RespirationClassifier::get_bandpassed_and_reduced_signal(segment);
    
    //find out where possible respiration clusters are
    
    hmm_segmenter(projected_real_filtered_signal,live_signal,&_respiration_clusters[0]);
    
    std::cout << "inhaled: " <<  _respiration_clusters[inhaled] << std::endl;
    std::cout << "exhaled: " <<  _respiration_clusters[exhaled] << std::endl;
    
    
    Complex_t dx = _respiration_clusters[exhaled] - _respiration_clusters[inhaled];
    
    _variance = (dx.real()*dx.real() + dx.imag()*dx.imag()) / 16.0;
    
}


RespirationPrediction RespirationSegmenter::predict_respiration_state(const Eigen::MatrixXcf & transformed_frame, const float sample_rate_hz) {
    
    
    //TODO keep history of last N transformed_frames, and notice if we are transitioning from exhaled to inhaled
    
    RespirationPrediction pred;
    
    pred.respiration_probs = eval_pdfs(transformed_frame);
    
    for (int istate = 0; istate < NUM_RESPIRATION_STATES; istate++) {
        pred.respiration_probs_deriv[istate] = (pred.respiration_probs[istate] - _prev_prediction.respiration_probs[istate]) * sample_rate_hz;
    }
    
    _prev_prediction = pred;
    
    //maybe?  doubly pulsed
    //pred.inhaleexhale = pred.respiration_probs_deriv[exhaled] - pred.respiration_probs_deriv[inhaled];
    
    //the safe option (inhaling and exhaling should produce a signal)
    pred.inhaleexhale = pred.respiration_probs[inhaling] + pred.respiration_probs[exhaling]; //total movement
    
    return pred;
}


RespirationStateFloatArray_t RespirationSegmenter::eval_pdfs(const Eigen::MatrixXcf & transformed_frame) {
    
    
    
    RespirationStateFloatArray_t result;
    RespirationStateFloatArray_t zero_result;

    memset(zero_result.data(),0,result.size()*sizeof(float));

    if (_variance <= 1e-6) {
        return zero_result;
    }
    
    //basically assign a gaussian pdf to each state, centered at cluster, compute log likelihood
    //use smaller variance for exhaled and inhaled
    for (int istate = 0; istate < NUM_RESPIRATION_STATES; istate++) {
        Complex_t dx = transformed_frame(0,0) - _respiration_clusters[istate];
        float dxsquared = dx.real()*dx.real() + dx.imag()*dx.imag();;
        float var = _variance;
        
        if (istate == exhaled || istate == inhaled) {
            var /= 4.0;
        }
        
        result[istate] = -dxsquared / (2.0f * var) - 0.5*log(2*M_PI*var);
        

    }
    
    //turn from log likelihood to normalized probabilities, without fucking up the numerics (sometimes pdf evals are, really unlikely)
    float the_min = result[0];
    for (int istate = 1; istate < NUM_RESPIRATION_STATES; istate++) {
        if (result[istate] < the_min) {
            the_min = result[istate];
        }
    }
    
    for (int istate = 0; istate < NUM_RESPIRATION_STATES; istate++) {
        result[istate] -= the_min;
    }
    
    for (int istate = 0; istate < NUM_RESPIRATION_STATES; istate++) {
        result[istate] = exp(result[istate]);
    }
    
    float sum = 0.0f;
    for (int istate = 0; istate < NUM_RESPIRATION_STATES; istate++) {
        sum += result[istate];
    }
    
    if (sum <= 1e-6) {
        return zero_result;
    }
    
    for (int istate = 0; istate < NUM_RESPIRATION_STATES; istate++) {
        result[istate] /= sum;
    }
    
    return result;

}

