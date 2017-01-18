#include "matlabreader.h"
#include "matio.h"
#include <assert.h>
#include <iostream>
#include <iomanip>
#include <complex>
#include <cmath>

using Eigen::MatrixXcf;
using namespace std::literals::complex_literals;

bool MatlabReader::read_baseband_from_file_v1(const std::string & filename, MatrixXcf & output) {
    
    mat_t * matfile = Mat_Open(filename.c_str(), MAT_ACC_RDONLY);
    
    if(matfile == NULL) {
        return false;
    }
    
    //matvar_t * p = Mat_VarReadNext(matfile);
    matvar_t * p = Mat_VarRead(matfile, "RecFrames");
    
    if (p == NULL) {
        return false;
    }
    
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
    
    if (frames == NULL) {
        return false;
    }
    
    if (frames->rank != 2) {
        return false;
    }
    
    const int num_rows = frames->dims[0];
    const int num_cols = frames->dims[1];
    
    const mat_complex_split_t *complex_data = static_cast<mat_complex_split_t *> (frames->data);
    const size_t N = frames->nbytes / frames->data_size;
    
    assert(N == num_rows * num_cols);
    
    if (frames->class_type == MAT_C_DOUBLE) {
        
        const double * re = (double *)(complex_data->Re);
        const double * im = (double *)(complex_data->Im);

        MatrixXcf mat(num_rows,num_cols);
        
        for (int k = 0; k < N; k++) {
            const int irow = k/num_cols;
            const int icol = k % num_cols;
            mat(irow,icol) = std::complex<float>(re[k],im[k]);
        }
        
        output = mat;
        
        return true;
    }
    
    if (frames->class_type == MAT_C_SINGLE) {
    
        const float * re = (float *)(complex_data->Re);
        const float * im = (float *)(complex_data->Im);
        
        MatrixXcf mat(num_rows,num_cols);
        
        for (int k = 0; k < N; k++) {
            const int irow = k/num_cols;
            const int icol = k % num_cols;
            
            mat(irow,icol) = std::complex<float>(re[k],im[k]);
        }
        
        output = mat;

        return true;
    }
    
 
    return false;
    
    
    Mat_Close(matfile);
}
