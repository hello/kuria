#include "gtest/gtest.h"
#include "example.h"
#include "HaltijaMath.h"

class TestRadar1 : public ::testing::Test {
protected:
    
    
    virtual void SetUp() {
    }
    
    virtual void TearDown() {
    }
    
};

class DISABLED_TestRadar1 : public TestRadar1 {};


TEST_F(TestRadar1, TestSimpleExample) {
    complex_matrices();
    
    real_matrices();
}

TEST_F(TestRadar1,TestFFT) {
    //HaltijaMath::complex_fft(const int nfft, const Eigen::VectorXcf &x, Eigen::VectorXcf &output);
}
