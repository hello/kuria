#ifndef _PCA_H_
#define _PCA_H_

#include <Eigen/Core>

class Pca {
public:
	Pca(const Eigen::MatrixXcf & x);
	~Pca();

	void fit();

	Eigen::MatrixXcf getTransformedData() const;

	Eigen::MatrixXcf getEigenVectors() const;


private:
	bool _is_worked;
	Eigen::MatrixXcf _x;
	Eigen::MatrixXcf _eigenvalues;
	Eigen::MatrixXcf _eigenvectors;
};

#endif //_PCA_H_