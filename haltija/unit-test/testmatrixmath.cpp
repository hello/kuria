#include "gtest/gtest.h"
#include "example.h"
#include "pca.h"
#include "haltijamath.h"

using namespace Eigen;

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
    Eigen::MatrixXf transformed_values;

    
    ASSERT_TRUE(pca(A,principal_components,transform,transformed_values));
    
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

TEST_F(TestMatrixMath,TestPCAComplex) {
    Eigen::MatrixXcf A(10,2);
    A <<
    Complex_t(0.47837943,0.84938962),
    Complex_t(-1.83079997,2.3900644),
    Complex_t(-1.58943112,0.41812988),
    Complex_t(-1.91636042,2.67415351),
    Complex_t(2.07226756,3.39165796),
    Complex_t(0.27881882,3.25907788),
    Complex_t(0.04400696,1.02441223),
    Complex_t(-2.24373993,0.96957763),
    Complex_t(1.22039051,2.0936234),
    Complex_t(-4.58645149,3.54934144),
    Complex_t(1.63074863,-1.19770075),
    Complex_t(-1.23502301,2.83018114),
    Complex_t(-0.33796550,2.22497448),
    Complex_t(-3.49572277,1.95620416),
    Complex_t(3.95641312,2.00079622),
    Complex_t(-1.06304623,2.16188159),
    Complex_t(0.52027377,0.92429317),
    Complex_t(0.28508555,2.30775933),
    Complex_t(-0.66495270,-0.16302931),
    Complex_t(0.88713586,0.55588004);

    Eigen::MatrixXcf principal_components;
    Eigen::MatrixXcf transform;
    Eigen::MatrixXcf transformed_values;
    
    
    ASSERT_TRUE(pca(A,principal_components,transform,transformed_values));
    
    ASSERT_NEAR(principal_components(0,0).imag(),0,1e-5);
    ASSERT_NEAR(principal_components(1,0).imag(),0,1e-5);

    ASSERT_NEAR(principal_components(0,0).real(),2.50676344,1e-5);
    ASSERT_NEAR(principal_components(1,0).real(),4.74644158,1e-5);

     Eigen::MatrixXcf testmat = transformed_values.transpose() * transformed_values.conjugate();
    
    ASSERT_NEAR(testmat(0,1).real(),0,1e-5);
    ASSERT_NEAR(testmat(1,0).real(),0,1e-5);
    ASSERT_NEAR(testmat(0,1).imag(),0,1e-5);
    ASSERT_NEAR(testmat(1,0).imag(),0,1e-5);

  
    
}


TEST_F(TestMatrixMath,TestPCAComplexToReal) {
    MatrixXcf c = MatrixXcf::Zero(10,1);
    MatrixXcf c2 = MatrixXcf::Zero(10,2);
    
    c <<
    Complex_t(1.0,1.0),
    Complex_t(-1.0,-1.0),
    Complex_t(1.0,1.0),
    Complex_t(-1.0,-1.0),
    Complex_t(1.0,1.0),
    Complex_t(-1.0,-1.0),
    Complex_t(1.0,1.0),
    Complex_t(-1.0,-1.0),
    Complex_t(1.0,1.0),
    Complex_t(-1.0,-1.0);
    
    c2.col(0) = c;
    c2.col(1) = c;

    MatrixXf result = HaltijaMath::project_complex_cols_into_reals(c2);
    
    for (int i = 0; i < 10; i++) {
        ASSERT_NEAR(fabs(result(i,0)),sqrt(2),1e-4);
        ASSERT_NEAR(fabs(result(i,1)),sqrt(2),1e-4);
    }
    
    ASSERT_NEAR(result(0,0) + result(1,0),0,1e-4);
    ASSERT_NEAR(result(0,1) + result(1,1),0,1e-4);


}
