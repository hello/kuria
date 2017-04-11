#ifndef _PCA_H_
#define _PCA_H_

#include <Eigen/Core>
#include <Eigen/Dense>




//x is data in column format (i.e. each row is a feature vector)
template <class T>
class Pca {
public:
    
    void fit (const T & x) {
        T x_zero_mean = x;
        const int N = x.rows();
        
        if (N <= 0) {
            return;
        }
        
        //create covariance matrix
        for (int icol = 0; icol < x.cols(); icol++) {
            x_zero_mean.col(icol).array() -= (x.col(icol).array().sum() / (float)N);
        }
        
        auto covariance = (x_zero_mean.transpose() * x_zero_mean.conjugate()) / x.rows();
        
        Eigen::SelfAdjointEigenSolver<T> eigensolver(covariance);
        
        //   S = EIGEN_VEC * EIGEN_VAL * EIGEN_VEC'
        //   [T x N]  * [N x N] ====> [T x N]
        _principal_components = eigensolver.eigenvalues();
        _transform = eigensolver.eigenvectors();
        
    }
    
    const T & get_transform() const {
        return _transform;
    }
    
    
    const T & get_principal_components() const {
        return _principal_components;
    }
    
    T get_transformed_values(const T & x) {
        T x_zero_mean = x;
        const int N = x.rows();
        
        if (N <= 0) {
            return x;
        }
        
        //create covariance matrix
        for (int icol = 0; icol < x.cols(); icol++) {
            x_zero_mean.col(icol).array() -= (x.col(icol).array().sum() / (float)N);
        }

        return x_zero_mean * _transform.conjugate();

    }
    
private:
    
    T _principal_components;
    T _transform;

};






#endif //_PCA_H_
