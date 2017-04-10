#ifndef _EKF_H_
#define _EKF_H_

#include <Eigen/Core>

class ExtendedKalmanFilter {
public:
    
    void initialize(Eigen::VectorXf initial_state, Eigen::MatrixXf covariance_matrix);
    
    virtual Eigen::MatrixXf getStateTransitionMatrix() const = 0;
    virtual Eigen::MatrixXf getObservationTransform() const = 0;

protected:
    
    Eigen::VectorXf _state;
    Eigen::MatrixXf _covariance_matrix;
    
};


#endif //_EKF_H_
