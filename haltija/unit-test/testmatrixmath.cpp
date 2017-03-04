#include "gtest/gtest.h"
#include "example.h"
#include "pca.h"

class TestMatrixMath : public ::testing::Test {
protected:
    
    
    virtual void SetUp() {
    }
    
    virtual void TearDown() {
    }
    
};

class DISABLED_TestMatrixMath : public TestMatrixMath {};



TEST_F(TestMatrixMath,TestPCAReals) {
    //HaltijaMath::complex_fft(const int nfft, const Eigen::VectorXcf &x, Eigen::VectorXcf &output);
    
    Eigen::MatrixXf A(3,2);
    A <<  1.17025654, -0.28766909,0.13481309, -0.43398725,0.98603634, -1.44139354;
    //std::cout << A << std::endl << std::endl;

    Eigen::MatrixXf principal_components;
    Eigen::MatrixXf transform;
    
    
    ASSERT_TRUE(pca(A,principal_components,transform));
    
    //std::cout << principal_components << std::endl << std::endl;
    //std::cout << transform << std::endl;

    ASSERT_EQ(principal_components.rows(),2);
    ASSERT_EQ(principal_components.cols(),1);
    ASSERT_EQ(transform.rows(),2);
    ASSERT_EQ(transform.cols(),2);
    
    //matches python PCA values -- some worry if the eigenvalues will be ascending or descending
    ASSERT_NEAR(principal_components(0,0),0.17080912,1e-4);
    ASSERT_NEAR(principal_components(1,0),0.2956373,1e-4);

}

