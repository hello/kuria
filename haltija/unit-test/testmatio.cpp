#include "gtest/gtest.h"
#include "example.h"
#include "matio/matio.h"

class TestMatio : public ::testing::Test {
protected:
    
    
    virtual void SetUp() {
    }
    
    virtual void TearDown() {
    }
    
};

class DISABLED_TestMatio : public TestMatio {};


TEST_F(TestMatio, OpenAndClose) {
	mat_t * matfile = Mat_Open("RecX4_BB_Frames_20161028_170759.mat", MAT_ACC_RDONLY);
	ASSERT_TRUE(matfile != NULL);
	Mat_Close(matfile);
}


TEST_F(TestMatio, TestReadingStuff) {
	mat_t * matfile = Mat_Open("RecX4_BB_Frames_20161028_170759.mat", MAT_ACC_RDONLY);

	ASSERT_TRUE(matfile != NULL);

	matvar_t * var = Mat_VarReadNext(matfile);

	std::cout << Mat_VarGetSize(var) << std::endl;


	Mat_Close(matfile);
}
