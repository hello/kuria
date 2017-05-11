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

#define PEAK_TO_PEAK_TIME_SECONDS_LOW_THRESHOLD (1.0)
#define PEAK_TO_PEAK_TIME_SECONDS_HIGH_THRESHOLD (6.0)


/* extract respiration frequency range, assume small phase changes -- and project complex numbers into reals
  via principal component analysis */
Eigen::MatrixXf RespirationClassifier::get_bandpassed_and_reduced_signal(const Eigen::MatrixXcf & probable_respiration_linear_combinations) {
    
    MatrixXf B(3,1);
    MatrixXf A(3,1);
    //B,A = sig.iirdesign(wp=[0.2/10.0,1.0/10.0],ws=[0.1/10.0,4.0 / 10.0],gpass=1.0,gstop = 6.0,ftype='butter')
    B << 0.21856269,  0.        , -0.21856269;
    A << 1.        , -1.55511674,  0.56287462;
    
    IIRFilter<MatrixXf, MatrixXf> bandpass_filter(B,A,1);
    
    const int idx = probable_respiration_linear_combinations.cols() - 1;
    
    
    return HaltijaMath::project_complex_cols_into_reals(probable_respiration_linear_combinations.col(idx));
    
}


RespirationStats RespirationClassifier::get_respiration_stats(const Eigen::MatrixXcf & probable_respiration_linear_combinations, const float sample_rate_hz) {

    
    
    const Eigen::MatrixXf projected_real_filtered_signal = get_bandpassed_and_reduced_signal(probable_respiration_linear_combinations);

    
    debug_save("bandpassed",projected_real_filtered_signal);

    
    const int T = projected_real_filtered_signal.rows();
    
    IntVec_t positive_crossings;
    IntVec_t negative_crossings;
    
    
    for (int t = 1; t < T; t++) {
        float prev = projected_real_filtered_signal(t - 1,0);
        float current = projected_real_filtered_signal(t,0);
        
        if (prev < 0.0f && current >= 0.0f) {
            positive_crossings.push_back(t);
        }
        
        if (prev >= 0.0f && current < 0.0f) {
            negative_crossings.push_back(t);
        }
    }
    
    float energy = 0.0;
    for (int t = 0; t < T; t++) {
        energy += projected_real_filtered_signal(t,0) * projected_real_filtered_signal(t,0);
    }
    
    IntVec_t diffs;
    
    for (int i = 1; i < positive_crossings.size(); i++) {
        diffs.push_back(positive_crossings[i] - positive_crossings[i-1]);
    }
    
    for (int i = 1; i < negative_crossings.size(); i++) {
        diffs.push_back(negative_crossings[i] - negative_crossings[i-1]);
    }
    
    if (diffs.empty()) {
        return RespirationStats(10*log10(energy));
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
    
    return RespirationStats(mean,stddev,10*log10(energy),is_possible_respiration);
     
}
