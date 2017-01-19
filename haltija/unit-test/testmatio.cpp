#include "gtest/gtest.h"
#include "example.h"
#include "matio.h"
#include "matlabreader.h"
#include "matlabwriter.h"

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

    matvar_t * p = Mat_VarReadNext(matfile);
    //matvar_t * p = Mat_VarRead(matfile, "RecFrames");
    
    ASSERT_TRUE(p != NULL);

    /*
    {
        char * const * field_names = Mat_VarGetStructFieldnames(p);
        const int n = Mat_VarGetNumberOfFields(p);
        for (int i = 0; i < n; i++) {
            std::cout << field_names[i] << std::endl;
        }
    }
     */
    
    matvar_t * frames = Mat_VarGetStructFieldByName(p,"Frames",0);
    
    ASSERT_TRUE(frames);
    ASSERT_TRUE (frames->class_type == MAT_C_DOUBLE);
    
    const mat_complex_split_t *complex_data = static_cast<mat_complex_split_t *> (frames->data);
    const double * re = (double *)(complex_data->Re);
    const double * im = (double *)(complex_data->Im);
    const size_t N = frames->nbytes / frames->data_size;
    /*
    for (int i = 0; i <  N; i++) {
        std::cout << re[i] << "," << im[i] << std::endl;
    }
     */
    
    ASSERT_TRUE(re[N-1] != 0.0);
    ASSERT_TRUE(im[N-1] != 0.0);


	Mat_Close(matfile);
}

TEST_F(TestMatio,TestReadBaseband) {
    std::string filepath = GET_TEST_FILE_PATH("RecX4_BB_Frames_20161028_170759.mat");

    Eigen::MatrixXcf mat;
    ASSERT_TRUE(MatlabReader::read_baseband_from_file_v1(filepath,mat));
    
    ASSERT_TRUE(mat.rows() == 2099);
    ASSERT_TRUE(mat.cols() == 188);
    
    //std::cout << mat(2098,187) << std::endl;
    std::complex<float> x = mat(2098,187);
    ASSERT_NEAR(x.real(),-218637,1e-3);
    ASSERT_NEAR(x.imag(),-479228,1e-3);


}

TEST_F(TestMatio,TestWriteMatrix) {
    
    ASSERT_TRUE(MatlabWriter::get_instance()->open_new_matfile("./foobars.mat"));
    
    MatrixXf mat(2,3);
    mat << 1,2,3,4,5,6;
    
    ASSERT_TRUE(MatlabWriter::get_instance()->write_real_matrix("hellothere", mat));
    
    MatrixXf mat2(3,2);
    mat << 7,8,9,10,11,12;
    ASSERT_TRUE(MatlabWriter::get_instance()->write_real_matrix("kthxbye", mat2));
    
    MatlabWriter::get_instance()->close();

    
    
    
}

