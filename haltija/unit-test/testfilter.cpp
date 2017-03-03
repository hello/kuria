
#include "gtest/gtest.h"
#include "example.h"
#include "filters.h"
#include <Eigen/Core>

using namespace Eigen;

class TestFilter : public ::testing::Test {
protected:
    
    
    virtual void SetUp() {
    }
    
    virtual void TearDown() {
    }
    
};

class DISABLED_TestFilter: public TestFilter {};


TEST_F(TestFilter, TestVecFIRFilter) {
    MatrixXf B(3,1);
    B << 1.0/3.0,1.0/3.0,1.0/3.0;
    
    MatrixXf x(4,1);
    x << 1.0,2.0,3.0,4.0;
    
    MatrixXf y = fir_filter_columns(B,x);
    
    ASSERT_EQ(y.size(),2);
    ASSERT_NEAR(y(0),2.0,1e-5);
    ASSERT_NEAR(y(1),3.0,1e-5);
}


TEST_F(TestFilter, TestMatFIRFilter) {
    MatrixXf B(3,1);
    B << 1.0/3.0,1.0/3.0,1.0/3.0;
    
    MatrixXf x(4,2);
    x << 1.0,2.0,
         3.0,4.0,
         5.0,6.0,
         7.0,8.0;

    
    
    MatrixXf y = fir_filter_columns(B,x);
    MatrixXf y2 = fir_filter_columns(B,x,1);

    ASSERT_EQ(y.size(),2);
    ASSERT_NEAR(y(0),3.0,1e-5);
    ASSERT_NEAR(y(1),5.0,1e-5);
    ASSERT_NEAR(y2(0),4.0,1e-5);
    ASSERT_NEAR(y2(1),6.0,1e-5);
    
}


