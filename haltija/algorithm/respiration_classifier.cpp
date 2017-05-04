 #include "respiration_classifier.h"
#include <float.h>
#include "haltijamath.h"
#include "debug_publisher.h"
#include "filters.h"
#include "haltijamath.h"
#include "hmm/HmmHelpers.h"
#include "hmm/AllModels.h"


/*
 
 breaths per minute
 
 Newborn to 6 months	30-60  (1 - 0.5 Hz)
 6 to 12 months	24-30
 1 to 5 years	20-30
 6 to 12 years	12-20  (1/5 to 1/3 hz)
 
 so basically anythign less than 1 hz
 
 
 
 
 */

using Eigen::MatrixXcf;
using Eigen::MatrixXf;

//these are in hz
#define FREQUENCY_CUTOFF (1.1f)
#define PEAK_FREQ_DIFFERENCE_HZ (0.1f)

//these are all in dB power
#define PEAK_ENERGY_ABOVE_MEAN (10.0f)
#define POS_NEG_FREQ_ENERGY_CONSISTENCY (6.0f)

#define PEAK_TO_PEAK_TIME_SECONDS_LOW_THRESHOLD (1.0)
#define PEAK_TO_PEAK_TIME_SECONDS_HIGH_THRESHOLD (6.0)


/*
   Basic idea: measurement model is stationary, changing
 
 
 */


void hmm_segmenter(const MatrixXf & x,const Eigen::MatrixXcf & orig, Complex_t  * resipration_clusters) {
    
    
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


RespirationStats RespirationClassifier::get_respiration_stats(const Eigen::MatrixXcf & probable_respiration_linear_combinations, const float sample_rate_hz) {

    MatrixXf B(3,1);
    MatrixXf A(3,1);
    //B,A = sig.iirdesign(wp=[0.2/10.0,1.0/10.0],ws=[0.1/10.0,4.0 / 10.0],gpass=1.0,gstop = 6.0,ftype='butter')
    B << 0.21856269,  0.        , -0.21856269;
    A << 1.        , -1.55511674,  0.56287462;
    
    IIRFilter<MatrixXf, MatrixXf> bandpass_filter(B,A,1);
    
    const int idx = probable_respiration_linear_combinations.cols() - 1;

    
    const Eigen::MatrixXf projected_real_signal = HaltijaMath::project_complex_cols_into_reals(probable_respiration_linear_combinations.col(idx));

    
    const Eigen::MatrixXf filtered_signal = bandpass_filter.filtfilt(projected_real_signal);
    
    debug_save("orig",projected_real_signal);
    debug_save("bandpassed",filtered_signal);

    
    const int T = filtered_signal.rows();
    
    IntVec_t positive_crossings;
    IntVec_t negative_crossings;
    
    for (int t = 1; t < T; t++) {
        float prev = filtered_signal(t - 1,0);
        float current = filtered_signal(t,0);
        
        if (prev < 0.0f && current >= 0.0f) {
            positive_crossings.push_back(t);
        }
        
        if (prev >= 0.0f && current < 0.0f) {
            negative_crossings.push_back(t);
        }
    }
    
    IntVec_t diffs;
    
    for (int i = 1; i < positive_crossings.size(); i++) {
        diffs.push_back(positive_crossings[i] - positive_crossings[i-1]);
    }
    
    for (int i = 1; i < negative_crossings.size(); i++) {
        diffs.push_back(negative_crossings[i] - negative_crossings[i-1]);
    }
    
    if (diffs.empty()) {
        return RespirationStats();
    }
    
    float sum = 0.0;
    for (auto it = diffs.begin(); it != diffs.end(); it++) {
        sum += *it;
    }
    
    float mean = sum / (float)diffs.size();
    float var = 0.0;
    for (auto it = diffs.begin(); it != diffs.end(); it++) {
        var += (*it - mean)*(*it - mean);
    }
    
    var /= (float)diffs.size();
    float stddev = sqrt(var);

    stddev /= sample_rate_hz;
    mean /= sample_rate_hz;
    
    bool is_possible_respiration = true;


    if (mean < PEAK_TO_PEAK_TIME_SECONDS_LOW_THRESHOLD) {
        is_possible_respiration = false;
    }

    if (mean > PEAK_TO_PEAK_TIME_SECONDS_HIGH_THRESHOLD) {
        is_possible_respiration = false;
    }
    
    if (2*stddev > mean) {
        is_possible_respiration = false;
    }
    
    //find out where possible respiration clusters are
    Complex_t respiration_clusters[NUM_RESPIRATION_STATES];
    
    hmm_segmenter(filtered_signal,probable_respiration_linear_combinations.col(idx),&respiration_clusters[0]);
    
    std::cout << "inhaled: " <<  respiration_clusters[inhaled] << std::endl;
    std::cout << "exhaled: " <<  respiration_clusters[exhaled] << std::endl;

    return RespirationStats(mean,stddev,is_possible_respiration,respiration_clusters);
     
}
