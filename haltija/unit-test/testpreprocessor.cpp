#include "gtest/gtest.h"
#include "preprocessor.h"
#include "matlabreader.h"

using Eigen::MatrixXcf;
using namespace MatlabReader;

#define GET_TEST_FILE_PATH(x)\
    UNIT_TEST_DATA + std::string(x)

class TestPreprocessor : public ::testing::Test {
protected:
    
    
    virtual void SetUp() {
        idx = -1;
    }
    
    virtual void TearDown() {
        if (idx >= 0) {
            
        }
    }
    
    int idx;
    
};

class DISABLED_TestPreprocessor : public TestPreprocessor {};


TEST_F(TestPreprocessor, Test1) {
    const std::string test_input_fname = GET_TEST_FILE_PATH("framesBeforeMTI.mat");
    
    const std::string ref_fname = GET_TEST_FILE_PATH("framesAfterMTIPython.mat");
    
    MatrixXcf input;
    ASSERT_TRUE(read_complex_matrix(test_input_fname,"framesBeforeMTI",input));
    
    MatrixXcf ref;
    ASSERT_TRUE(read_complex_matrix(ref_fname,"ref_out",ref));
    
    auto p = Preprocessor::createWithDefaultHighpassFilter(input.cols(),1,1);
    
    int irow2 = 0;
    
    for (int irow = 0; irow < input.rows(); irow++) {
        BasebandDataFrame_t frame;
        frame.timestamp = irow;
        frame.data.reserve(input.cols());
        
        for (int icol = 0; icol < input.cols(); icol++) {
            frame.data.push_back(input(irow,icol));
        }
        
        MatrixXcf filtered_frame;
        MatrixXcf segment;
        
        uint32_t flags = p->add_frame(frame, filtered_frame, segment);
        
        
        if ( ~flags & PREPROCESSOR_FLAGS_FRAME_READY) {
            continue;
        }
        
        
        const int nrangebins = input.cols();
        for (int icol = 0; icol < nrangebins; icol++) {
            //std::cout << icol << ":" << filtered_frame(0,icol).real() << "," << ref(irow2,icol).real() << std::endl;
            
            ASSERT_NEAR(filtered_frame(0,icol).real(),ref(irow2,icol).real(),1e-2);
            ASSERT_NEAR(filtered_frame(0,icol).imag(),ref(irow2,icol).imag(),1e-2);
        }
        
        irow2++;
    }
    

    
}

