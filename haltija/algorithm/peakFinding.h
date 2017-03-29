//
//  peakFinding.h
//  Haltija
//
//  Created by Mari Zakrzewski on 3/21/17.
//
//

#ifndef peakFinding_h
#define peakFinding_h

#include <Eigen/Core>
#include <list>
#include "filters.h"
#include <memory>

//#include <vector> // needed?
//#include <stdio.h>


class Peakfinder  {
public:
    Peakfinder();
    ~Peakfinder();
    bool isPeak(const Eigen::MatrixXcf & transformed_frame, const int iframe, Eigen::MatrixXf & filtered_sample);
    void lpFilter(const Eigen::MatrixXcf & transformed_frame, Eigen::MatrixXf & filtered_sample); // could be also private func
  
    // functions to calculate respiration rate (RR)
    //float getInstantRR();
    //float getMeanRR(); // over lates 10 sec.?
    
private:
    
    std::list<float> _maxima;
    std::list<float> _minima;
    std::list<int> _maximaind;
    std::list<int> _minimaind;
    
    float _prev_sample;
    float _prev_deriv;
    
    typedef std::shared_ptr<IIRFilter<Eigen::MatrixXf, Eigen::MatrixXf>> FloatIIRSharedPtr_t;
    FloatIIRSharedPtr_t _lpf;
    
};

#endif /* peakFinding_h */


