#include "gtest/gtest.h"
#include "example.h"
#include "matio.h"
#include "matlabreader.h"
#include "matlabwriter.h"
#include <istream>
#include <fstream>
#include <regex>

typedef std::complex<float> Complex_t;
#define SCIENTIFIC_REGEX "[-+]?[0-9]*\.?[0-9]+([eE][-+]?[0-9]+)?i?"
class TestMatio : public ::testing::Test {
protected:
    
    
    virtual void SetUp() {
    }
    
    virtual void TearDown() {
    }
    
    static Eigen::MatrixXcf read_complex_csv(const std::string & filepath) {
        
        const std::regex e (SCIENTIFIC_REGEX);
        
       
        
        
        std::ifstream csvfile(filepath);
        std::vector<Complex_t> vec;
        vec.reserve(1000000);
        
        std::string line;
        int nrows = 0;
        int count = 0;
        int ncols = 0;
        
        while(std::getline(csvfile, line)) {
            nrows++;
            std::string item;
            std::stringstream ss;
            ss << line;
            while (std::getline(ss,item,',')  ) {
                count++;

                std::smatch m;
                std::string s = item;
                int icomplex = 0;
                std::string cmplx[2] = {"0.0","0.0i"};
                
                while (std::regex_search (s,m,e)) {
                    if (!m.empty()) {
                        std::string result = m[0];
                        
                        if (result.empty()) {
                            break;
                        }
                        
                        if (result[result.size()-1] == 'i' || result[result.size()-1] == 'j') {
                            result.pop_back();
                        }
                        
                        cmplx[icomplex++] = result;
                    }
                    
                    s = m.suffix().str();
                }
                
                std::string number = "(" + cmplx[0] + "," + cmplx[1] + ")";
                std::istringstream is(number);
                Complex_t c;
                is >> c;
                vec.push_back(c);
                
            }
            
            if (!ncols) {
                ncols = count;
            }
            
            
        }
        
        Eigen::MatrixXcf mat(nrows,ncols);
        
        for (int j = 0; j < nrows; j++) {
            for (int i = 0; i < ncols; i++) {
                mat(j,i) = vec[j*ncols + i];
            }
        }

        
        return mat;
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

TEST_F(TestMatio,TestReadBasebandVsReference) {
    std::string filepath = GET_TEST_FILE_PATH("RecX4_BB_Frames_subset.mat");
    std::string refpath = GET_TEST_FILE_PATH("RecFrames_subset.csv");

    Eigen::MatrixXcf refmat = read_complex_csv(refpath);
    
    Eigen::MatrixXcf mat;
    ASSERT_TRUE(MatlabReader::read_baseband_from_file_v1(filepath,mat));

    ASSERT_TRUE(mat.rows() == 50);
    ASSERT_TRUE(mat.cols() == 50);
    ASSERT_TRUE(refmat.rows() == 50);
    ASSERT_TRUE(refmat.cols() == 50);
    
    for (int j = 0; j < mat.rows(); j++) {
        for (int i = 0; i < mat.cols(); i++) {
            
            Complex_t diff = mat(j,i) - refmat(j,i);
            
            Complex_t denom = mat(j,i) + refmat(j,i);
            
            
            if (fabs(denom.real()) < 1e-3) {
                denom.real(1e-3);
            }
            
            if (fabs(denom.imag()) < 1e-3) {
                denom.imag(1e-3);
            }

            Complex_t frac = diff / denom;
            
            ASSERT_LE(fabs(frac.real()),1e-4);
            ASSERT_LE(fabs(frac.imag()),1e-4);

        }
    }

    
}

TEST_F(DISABLED_TestMatio,TestWriteMatrixReals) {
    
    ASSERT_TRUE(MatlabWriter::get_instance()->open_new_matfile("reals.mat"));
    
    MatrixXf mat(2,3);
    mat << 1,2,3,4,5,6;
    
    ASSERT_TRUE(MatlabWriter::get_instance()->write_real_matrix("hellothere", mat));
    
    MatrixXf mat2(3,2);
    mat << 7,8,9,10,11,12;
    ASSERT_TRUE(MatlabWriter::get_instance()->write_real_matrix("kthxbye", mat2));
    
    MatlabWriter::get_instance()->close();
}

TEST_F(DISABLED_TestMatio,TestWriteMatrixComplex) {
    
    ASSERT_TRUE(MatlabWriter::get_instance()->open_new_matfile("complex.mat"));
    
    MatrixXcf mat(2,3);
    mat << Complex_t(1.0,1.0),Complex_t(2.0,2.0),Complex_t(3.0,3.0),Complex_t(4.0,-4.0),Complex_t(5.0,5.0),Complex_t(6.0,-6.0);
    
    ASSERT_TRUE(MatlabWriter::get_instance()->write_complex_matrix("c1", mat));
    
    
    MatlabWriter::get_instance()->close();
    
    
}



