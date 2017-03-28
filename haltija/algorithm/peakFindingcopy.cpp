//
//  peakFinding.cpp
//  Haltija
//
//  Created by Mari Zakrzewski on 3/21/17.
//
//

#include <iostream>
#include "peakFinding.h"
#include "filters.h"
#include "haltija_types.h"
#include <list>

using namespace Eigen;

Peakfinder::Peakfinder()
: _maxima()
, _minima()
, _maximaind()
, _minimaind()
{
    _prev_sample = 0;
    _prev_deriv = 0;

    
}

Peakfinder::~Peakfinder() {
}


bool Peakfinder::isPeak(const MatrixXcf & transformed_sample, const int iframe, MatrixXf & filtered_sample) {
    // Filter first
    //MatrixXf filtered_sample;
    lpFilter(transformed_sample, filtered_sample);
    
    float deriv;
    deriv = filtered_sample(0,0) - _prev_sample;
    //std::cout << "deriv " << deriv << std::endl;
    //std::cout << "prev_deriv " << _prev_deriv << std::endl;

    
    if ((_prev_deriv < 0) && (deriv > 0) ) {
        _minima.push_front(_prev_sample);
        _minimaind.push_front(iframe - 1);

        // save the current sample for the next round
        _prev_sample = filtered_sample(0,0);
        _prev_deriv = deriv;
        //std::cout << "minima " << std::endl;
        return true;

    }
    else if (_prev_deriv > 0 && deriv < 0 ) {
        _maxima.push_front(_prev_sample);
        _maximaind.push_front(iframe - 1);
        
        // save the current sample for the next round
        _prev_sample = filtered_sample(0,0);
        _prev_deriv = deriv;
        //std::cout << "maxima " << std::endl;
        return true;
    }
    
    //if temppi2(i) < 0 && temppi2(i+1) > 0
    //Lets make sure, that there is no consecutive minimas
    /*if isempty(minimas_ind) || isempty(maximas_ind) || minimas_ind(end) < maximas_ind(end)
     minimas_ind = [minimas_ind; i];
     end
     */
    
    //No consecutive maximas
    //elseif temppi2(i) > 0 && temppi2(i+1) < 0
    //if isempty(minimas_ind) || isempty(maximas_ind) || maximas_ind(end) < minimas_ind(end)
    //maximas_ind = [maximas_ind; i];
    //end
    //end
    
    // save the current sample for the next round
    _prev_sample = filtered_sample(0,0);
    _prev_deriv = deriv;
    //std::cout << "prev_deriv " << _prev_deriv << std::endl;
    return false;
}

// First filter with low pass filter before running peak detection
void Peakfinder::lpFilter(const MatrixXcf & transformed_sample, MatrixXf & filtered_sample) {

    MatrixXf B(3,1);
    MatrixXf A(3,1);

    //B << 0.85284624, -1.70569249,  0.85284624;
    //A << 1.,         -1.68391975,  0.72746523;
    A << 1.000000000000000, -1.705552145544084, 0.743655195048866;
    B << 0.009525762376195,  0.019051524752390, 0.009525762376195;

    
    IIRFilter<MatrixXf, MatrixXf> f(B,A,1);
    
    MatrixXf temp(1,1);
    // add power calculation here!
    // something like
    // temp = sqrt( transformed_frame.real() .^2 + transformed_frame.imag() .^2 );
    temp = transformed_sample.real();
    //std::cout << "temp " << temp << std::endl;
    filtered_sample = f.filter(temp);
    
    return;
}



/*
 %% Find respiration extrema, i.e., peaks and valleys. Input should be real-valued data from
 % one range bin.
 %
 %theta = nx1
 %peakThreshold = minimum amplitude for a extrema, percentage of mean
 %amplitudes
 %amplitudesLH, amplitudesHL = Low-to-high, high-to-low, m x 1
 %iRR_pp, iRR_vv = instantaneous respiration "rate" in samples, peak-to-peak and
 %valley-to-valley time, m x 1
 %maximas_ind, minimas_ind = index of the minima or maxima (in samples)
 */
//function [new_amplitudesLH, new_amplitudesHL, amplitudesLH, amplitudesHL, iRR_pp, iRR_vv, ...
//          new_maximas_ind, new_minimas_ind, maximas_ind, minimas_ind, maxima, minima, temppi] = ...
//findRespirationExtrema(theta, fs, t, fig, peakThreshold, toPlot)

// peakThreshold = 0.2; % this value was used previously. Need to be reconsidered!!!

// Low pass filter heavily and find local minimas and maximas
//[temppi, groupDelay] = filterRespAndPlotSelectFilter(theta, t,1, 35, 'LP3', 0);  %2Hz OR 3Hz !!!!!!!!!!!!!!!!!!!!!!

// find local maxima
//maximas_ind = [];
//minimas_ind = [];

// Find the zero crossings of the derivatives
//temppi2 = diff(temppi);
//for i = 1:length(temppi) - 2
//if temppi2(i) < 0 && temppi2(i+1) > 0

//Lets make sure, that there is no consecutive minimas
/*if isempty(minimas_ind) || isempty(maximas_ind) || minimas_ind(end) < maximas_ind(end)
minimas_ind = [minimas_ind; i];
end
*/

//No consecutive maximas
//elseif temppi2(i) > 0 && temppi2(i+1) < 0
//if isempty(minimas_ind) || isempty(maximas_ind) || maximas_ind(end) < minimas_ind(end)
//maximas_ind = [maximas_ind; i];
//end
//end
//end

// Diff function shifts indexes one step to the left, the first values is
// the second - the first. Thus, add minimas and maximas indexes one.
//maximas_ind = maximas_ind + 1;
//minimas_ind = minimas_ind + 1;



/*
 //get first element out of sample
 Complex_t c = transformed_sample(0,0);
 float r = c.real();
 float i = c.imag();
 
 std::list<float> foo1;
 
 foo1.push_back(r);
 if (foo1.size() > 50) {
 foo1.pop_front();
 }
 
 float sum = 0.0;
 for (auto it = foo1.begin(); it != foo1.end(); it++) {
 sum += *it;
 }
 
 VectorXf myvec(10);
 myvec.dot(myvec);
 */


