#include "gtest/gtest.h"
#include "example.h"
#include "matio.h"

class TestMatio : public ::testing::Test {
protected:
    
    
    virtual void SetUp() {
    }
    
    virtual void TearDown() {
    }
    
};

class DISABLED_TestMatio : public TestMatio {};

#define GET_TEST_FILE_PATH(x)\
    UNIT_TEST_DATA + std::string(x)

TEST_F(TestMatio, OpenAndClose) {
	std::string filepath = GET_TEST_FILE_PATH("RecX4_BB_Frames_20161028_170759.mat");
	mat_t * matfile = Mat_Open(filepath.c_str(), MAT_ACC_RDONLY);
	ASSERT_TRUE(matfile != NULL);
	Mat_Close(matfile);
}


TEST_F(TestMatio, TestReadingStuff) {
	std::string filepath = GET_TEST_FILE_PATH("RecX4_BB_Frames_20161028_170759.mat");
	mat_t * matfile = Mat_Open(filepath.c_str(), MAT_ACC_RDONLY);

	ASSERT_TRUE(matfile != NULL);

    bool foundOne = false;
	 
    while(matvar_t * pNext = Mat_VarReadNext(matfile)) {
        std::cout << pNext->name << std::endl;
        foundOne = true;
    }
    
    ASSERT_TRUE(foundOne);
    
    Mat_Rewind(matfile);

    matvar_t * p = Mat_VarReadNext(matfile);
    
    ASSERT_TRUE(strcmp(p->name,"RecFrames") == 0);
    

    

	Mat_Close(matfile);
}

