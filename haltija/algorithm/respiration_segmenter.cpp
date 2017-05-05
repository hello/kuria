#include "respiration_segmenter.h"
#include "respiration_classifier.h"
#include <float.h>
#include "haltijamath.h"
#include "debug_publisher.h"
#include "filters.h"
#include "haltijamath.h"
#include "hmm/HmmHelpers.h"
#include "hmm/AllModels.h"
#include <assert.h>

using Eigen::MatrixXcf;
using Eigen::MatrixXf;


#define MIN_REL_LOG_LIKELIHOOD (-10.0f)

RespirationPrediction::RespirationPrediction() {
    memset(respiration_probs.data(),0,respiration_probs.size()*sizeof(float));
}


RespirationSegmenter::RespirationSegmenter() {
    
    /*
     1 + r + r^2 + r^3 .... r^inf = 1 / (1 - r)
     
     1 / (1 - r) = x
     1 - r = 1/x
     
     r = 1 - 1/x
     
     */
    
    
    const float long_term = 20.0f;
    const float short_term = 3.0f;
    
    const float a1 = 1.0f - 1.0f / long_term;
    const float b1 = 1.0f - a1;
    
    const float a2 = 1.0f - 1.0f / short_term;
    const float b2 = 1.0 - a2;
    
    _state_transition_matrix.resize(NUM_RESPIRATION_STATES,NUM_RESPIRATION_STATES);
    
    _state_transition_matrix <<
    a1,b1,0,0,
    0,a1,b1,0,
    0,0,a2,b2,
    b1,0,0,a1;
    
    assert(0 == exhaled);
    assert(1 == inhaling);
    assert(2 == inhaled);
    assert(3 == exhaling);
    
    /* use transpose for Bayes, because it is probability to transition INTO this state
     a1 0  0  b1
     b1 a1 0  0
     0  b1 a2 0
     0  0  b2 a1
     
     use original for HMM
     HMM follows convention of (current state is row i, prob to transition is row i, col j)
     i.e. transition OUT of this state
     */
    
    _state = 0.01 * Eigen::MatrixXf::Ones(NUM_RESPIRATION_STATES,1);
    
}

void RespirationSegmenter::hmm_segmenter(const MatrixXf & x,const Eigen::MatrixXcf & orig, Complex_t  * resipration_clusters) const {
    

    
    //create pdfs of states
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
    
  
    
    
    for (int j = 0; j < NUM_RESPIRATION_STATES; j++) {
        HmmDataVec_t v;
        for (int i = 0; i < NUM_RESPIRATION_STATES; i++) {
            v.push_back(_state_transition_matrix(j,i));
            
        }
        A.push_back(v);
    }

    
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




void RespirationSegmenter::set_segment(const Eigen::MatrixXcf segment, const Eigen::MatrixXcf live_signal,bool is_respiration) {
    
    //set to uniform, nevermind the scaling (gets normalized later
    if (!is_respiration) {
        _state = 0.01 * Eigen::MatrixXf::Ones(NUM_RESPIRATION_STATES,1);
        return;
    }
    
    //get real bandpassed signal from complex signal
    const Eigen::MatrixXf projected_real_filtered_signal = RespirationClassifier::get_bandpassed_and_reduced_signal(segment);
    
    //find out where possible respiration clusters are
    
    hmm_segmenter(projected_real_filtered_signal,live_signal,&_respiration_clusters[0]);
    
        /*
    std::cout << "exhaled: " <<  _respiration_clusters[exhaled] << std::endl;
    std::cout << "inhaling: " <<  _respiration_clusters[inhaling] << std::endl;
    std::cout << "inhaled: " <<  _respiration_clusters[inhaled] << std::endl;
    std::cout << "exhaling: " <<  _respiration_clusters[exhaling] << std::endl;
    */
    
    Complex_t dx = _respiration_clusters[exhaled] - _respiration_clusters[inhaled];
    
    _variance = (dx.real()*dx.real() + dx.imag()*dx.imag()) / 16.0;
    
}


RespirationPrediction RespirationSegmenter::predict_respiration_state(const Eigen::MatrixXcf & transformed_frame, const float sample_rate_hz) {
    
    
    bayes_update(transformed_frame);
    
    RespirationPrediction pred;

    for (int istate = 0; istate < NUM_RESPIRATION_STATES; istate++) {
        pred.respiration_probs[istate] = _state(istate,0);
    }
    
    return pred;
}


void RespirationSegmenter::bayes_update(const Eigen::MatrixXcf & transformed_frame) {
    
    
    
    RespirationStateFloatArray_t pdf_eval;
    
    //basically assign a gaussian pdf to each state, centered at cluster, compute log likelihood
    //use smaller variance for exhaled and inhaled
    for (int istate = 0; istate < NUM_RESPIRATION_STATES; istate++) {
        Complex_t dx = transformed_frame(0,0) - _respiration_clusters[istate];
        float dxsquared = dx.real()*dx.real() + dx.imag()*dx.imag();;
        float var = _variance;
        
        
        if (istate == exhaling || istate == inhaling) {
            var *= 4.0;
        }
        
        pdf_eval[istate] = -dxsquared / (2.0f * var) - 0.5*log(2*M_PI*var);
        
    }
    
    
    //turn from log likelihood to normalized probabilities, without fucking up the numerics (sometimes pdf evals are, really unlikely)
    float the_max = pdf_eval[0];

    for (int istate = 1; istate < NUM_RESPIRATION_STATES; istate++) {
        if (pdf_eval[istate] > the_max) {
            the_max = pdf_eval[istate];
        }
    }
    
    for (int istate = 0; istate < NUM_RESPIRATION_STATES; istate++) {
        pdf_eval[istate] -= the_max;
        
        if (pdf_eval[istate] < MIN_REL_LOG_LIKELIHOOD) {
            pdf_eval[istate] = MIN_REL_LOG_LIKELIHOOD;
        }
    }
    
    for (int istate = 0; istate < NUM_RESPIRATION_STATES; istate++) {
        pdf_eval[istate] = exp(pdf_eval[istate]);
    }
    
    
    //apply state transition matrix
    _state = _state_transition_matrix.transpose() * _state;
    _state /= _state.sum();
    
    
    //bayes rule, continuous discrete
    // p(A | x) = P(x | A) * P(A) / p(x)
    float px = 0.0f;
    for (int istate = 0; istate < NUM_RESPIRATION_STATES; istate++) {
        _state(istate,0) *=  pdf_eval[istate];
        px += _state(istate,0);
    }
    
    for (int istate = 0; istate < NUM_RESPIRATION_STATES; istate++) {
        _state(istate,0) /= px;
    }
    
}

