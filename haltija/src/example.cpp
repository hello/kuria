#include "example.h"
#include <Eigen/Dense>
#include <iostream>
#include <iomanip>
#include <complex>
#include <cmath>

using namespace std::literals::complex_literals;
using Eigen::MatrixXcf;
using Eigen::MatrixXf;

void complex_matrices() {
    MatrixXcf m(2,2);
    m(0,0) = 1.0 + 1.0i;
    m(1,0) = -1i;
    m(0,1) = -1.0 + 2.0i;
    m(1,1) = m(1,0) + m(0,1);
    
    std::cout << m << std::endl << std::endl;

    std::cout << m*m << std::endl;
}

void real_matrices() {
    MatrixXf m(2,2);

    m << 1.0,2.0,3.0,4.0;
    
    std::cout << m << std::endl;
    
}
