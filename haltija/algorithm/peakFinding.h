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
//#include <vector> // needed?
//#include <stdio.h>


using namespace Eigen;

class Peakfinder  {
public:
    Peakfinder();
    ~Peakfinder();
    bool isPeak(const MatrixXcf & transformed_frame, const int iframe, MatrixXf & filtered_sample);
    void lpFilter(const MatrixXcf & transformed_frame, MatrixXf & filtered_sample); // could be also private func
  
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
    
};

#endif /* peakFinding_h */


