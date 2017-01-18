#include "pca.h"
#include <Eigen/Dense>

using namespace Eigen;
typedef MatrixXcf ComplexMat;
typedef std::complex<float> ComplexVal;

//constructor
Pca::Pca(const Eigen::MatrixXcf & x) {
	_x = x;
	_is_worked = false;
}

//destructor
Pca::~Pca() {

}
/*
double do_float_math(double a, double b) {
	return a / b;
}

int do_integer_math(int a, int b) {
	return a / b;
}


template <class T> 
T do_all_math(T a, T b) {
	return a / b;
}

do_all_math<double>(a, b); equivalent to do_float_math
*/

void Pca::fit() {
	//get mean of data via rows
	// A = [[1 2];[3 4]]
	// y = A .* A
	// y2 = A * A
	//
	// in eigein
	// y2 = A*A  (works!)
	// y = A.array() * A.array()  
	// so array means element-wise operations
	ComplexMat x2 = _x;

	for (int irow = 0; irow < _x.rows(); irow++) {
		ComplexVal rowsum = _x.row(irow).array().sum();

		x2.row(irow) = x2.row(irow).array() - rowsum;
//		rowsum.real;
//		rowsum.imag;
	}

	ComplexMat covariance = x2.transpose()*x2;


	SelfAdjointEigenSolver<ComplexMat> eigensolver(covariance);
	ComplexMat transformed_values = eigensolver.eigenvectors() * x2;


}

Eigen::MatrixXcf Pca::getTransformedData() const {
	return ComplexMat::Zero(1, 1);
}

Eigen::MatrixXcf Pca::getEigenVectors() const {
	return ComplexMat::Zero(1, 1);
}