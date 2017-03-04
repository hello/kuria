#ifndef _PCA_H_
#define _PCA_H_

#include <Eigen/Core>
#include <Eigen/Dense>




//x is data in column format (i.e. each row is a feature vector)
template <class T,class R>
bool pca(const T & x, R & principal_components, R & transform) {
    
    T x_zero_mean = x;
    const int N = x.rows();
    
    if (N <= 0) {
        return false;
    }
    
    //create covariance matrix
    for (int icol = 0; icol < x.cols(); icol++) {
        x_zero_mean.col(icol).array() -= (x.col(icol).array().sum() / N);
    }
    
    auto covariance = (x_zero_mean.transpose() * x_zero_mean) / x.rows();
    
    Eigen::SelfAdjointEigenSolver<T> eigensolver(covariance);

    //transformed_values = x_zero_mean * eigensolver.eigenvectors().transpose();
    principal_components = eigensolver.eigenvalues();
    transform = eigensolver.eigenvectors();
    
    return true;
}



#endif //_PCA_H_
