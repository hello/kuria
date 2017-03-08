#include "gtest/gtest.h"
#include "preprocessor.h"

class TestPreprocessor : public ::testing::Test {
protected:
    
    
    virtual void SetUp() {
    }
    
    virtual void TearDown() {
    }
    
};

class DISABLED_TestPreprocessor : public TestPreprocessor {};


TEST_F(TestPreprocessor, Test1) {
   //TODO write a test
}

