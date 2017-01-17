#include "gtest/gtest.h"
#include "example.h"


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
