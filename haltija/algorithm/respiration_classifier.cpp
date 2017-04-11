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

static int find_arg_max(const int istart, const int iend, Eigen::VectorXf v) {
    float max = FLT_MIN;
    int imax = -1;
    
    for (int i = istart; i < iend; i++) {
        if (v(i) > max) {
            max = v(i);
            imax = i;
        }
    }
    
    return imax;
}

int RespirationClassifier::is_respiration(const Eigen::MatrixXcf & range_bins_of_interest, const float sample_rate_hz) {
    int nfft = 1 << ((int)log2(range_bins_of_interest.rows())  + 1 );
    const float cutoff_hz = FREQUENCY_CUTOFF; //hz
    const float hz_per_bin = sample_rate_hz / (float)nfft;
    const int cutoff_bin_pos = cutoff_hz / hz_per_bin;
    const int cutoff_bin_neg = nfft - cutoff_bin_pos;

    
    for (int icol = range_bins_of_interest.cols() - 1; icol >= 0 ; icol--) {
        
        Eigen::VectorXf result;
        Eigen::VectorXcf column = range_bins_of_interest.col(icol);
        
        Complex_t sum = column.sum();
        Complex_t mean(sum.real() / (float)column.rows(),sum.imag() / (float)column.rows());
       
        column.array() -= mean;
        
        
        HaltijaMath::psd(nfft,column,result);
        
        
        /*  debug */
        char buf[1024];
        memset(buf,0,sizeof(buf));
        snprintf(buf, sizeof(buf), "psd%03d",icol);
        std::string myid;
        myid.assign(buf);
        debug_save(myid,static_cast<Eigen::MatrixXf>(result));
        
        memset(buf,0,sizeof(buf));
        snprintf(buf, sizeof(buf), "orig%03d",icol);
        myid.assign(buf);
        debug_save(myid,static_cast<Eigen::MatrixXcf>(column));

        
        
        //calculate 1hz cutoff point
        
        //compute mean log energy
        float mean_energy = result.sum() / (float)result.rows();
        
        //find max in pos and negative;
        const int imaxlow = find_arg_max(1,nfft/2,result);
        const int imaxhigh = find_arg_max(nfft/2,nfft - 1,result);
        const int indices_diff = abs ( nfft - imaxhigh - imaxlow );
        const float indices_diff_hz = indices_diff * hz_per_bin;
        
        float energy_threshold = PEAK_ENERGY_ABOVE_MEAN + mean_energy;
        
        //check range bin of max energy positive frequencies
        if (imaxlow > cutoff_bin_pos) {
            continue;
        }
        
        //check range bin of max energy negative frequencies
        if (imaxhigh < cutoff_bin_neg) {
            continue;
        }
        
        //check energy amplitude
        if (result(imaxlow) < energy_threshold) {
            continue;
        }
        
        if (result(imaxhigh) < energy_threshold) {
            continue;
        }
        
        if (fabs(result(imaxhigh) - result(imaxlow)) > POS_NEG_FREQ_ENERGY_CONSISTENCY) {
            continue;
        }
        
        if (indices_diff_hz  > PEAK_FREQ_DIFFERENCE_HZ) {
            continue;
        }
        
        return icol;
    }
    
    return -1;
}

bool RespirationClassifier::get_respiration_stats(const Eigen::MatrixXcf & probable_respiration_linear_combinations, RespirationStats_t & result) {
    //TODO bandpass signal
    //TODO find indices of increasing zero crossings
    //TODO compute delta time of increasing zero crossings
    //TODO compute mean delta time, std dev of delta time
    
    MatrixXf B(5,1);
    MatrixXf A(5,1);
    //B,A = sig.iirdesign(wp=[0.2/10.0,1.0/10.0],ws=[0.1/10.0,4.0 / 10.0],gpass=1.0,gstop = 6.0,ftype='butter')
    B << 0.02446784,  0.        , -0.04893569,  0.        ,  0.02446784;
    A << 1.        , -3.47656882,  4.57040766, -2.70275474,  0.60922209;
    
    IIRFilter<MatrixXf, MatrixXf> bandpass_filter(B,A,probable_respiration_linear_combinations.cols());
    
    const int idx = 0;

    
    const Eigen::MatrixXf projected_real_signal = HaltijaMath::project_complex_cols_into_reals(probable_respiration_linear_combinations.col(idx));

    
    const Eigen::MatrixXf filtered_signal = bandpass_filter.filtfilt(projected_real_signal);
    
    const int T = filtered_signal.rows();
    
    IntVec_t positive_crossings;
    
    for (int t = 1; t < T; t++) {
        float prev = filtered_signal(t - 1,idx);
        float current = filtered_signal(t,idx);
        
        if (prev < 0.0f && current >= 0.0f) {
            positive_crossings.push_back(t);
        }
    }
    
    IntVec_t diffs;
    
    for (int i = 1; i < positive_crossings.size(); i++) {
        diffs.push_back(positive_crossings[i] - positive_crossings[i-1]);
    }
    
    return false;
}
