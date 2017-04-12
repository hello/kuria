 #include "respiration_classifier.h"
#include <float.h>
#include "haltijamath.h"
#include "debug_publisher.h"
#include "filters.h"
#include "haltijamath.h"


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

\

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
    
    return RespirationStats(mean,stddev,is_possible_respiration);
     
}
