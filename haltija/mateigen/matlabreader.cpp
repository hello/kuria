#include "matlabreader.h"
#include "matio.h"
#include <assert.h>
#include <iostream>
#include <iomanip>
#include <complex>
#include <cmath>

using Eigen::MatrixXcf;
using Eigen::MatrixXf;

using namespace std::literals::complex_literals;
typedef std::complex<float> Complex_t;

static bool read_real(matvar_t * var, Eigen::MatrixXf & output) {
    if (var == NULL) {
        return false;
    }
    
    if (var->rank != 2) {
        return false;
    }
    
    const int num_rows = var->dims[0];
    const int num_cols = var->dims[1];

    const int N = num_rows * num_cols;
    
    MatrixXf mat(num_rows,num_cols);

    switch (var->class_type) {
            
        case MAT_C_DOUBLE:
        {
            const double * re = (double *)(var->data);
            float * x = mat.data();
            
            for (int k = 0; k < N; k++) {
                x[k] = re[k];
            }
            
            output = mat;
            break;
        }
            
        case MAT_C_SINGLE:
        {
            const float * re = (float *)(var->data);
            float * x = mat.data();
            
            for (int k = 0; k < N; k++) {
                x[k] = re[k];
            }
            
            output = mat;
            break;
        }
            
            
            
        default:
        {
            return false;
        }
            
    }
    
    return true;

}

static bool read_complex(matvar_t * var, Eigen::MatrixXcf & output) {
    if (var == NULL) {
        return false;
    }
    
    if (var->rank != 2) {
        return false;
    }
    
    const int num_rows = var->dims[0];
    const int num_cols = var->dims[1];
    
    const mat_complex_split_t *complex_data = static_cast<mat_complex_split_t *> (var->data);
    const size_t N = var->nbytes / var->data_size;
    
    assert(N == num_rows * num_cols);
    
    //BOTH MATLAB AND EIGEN ARE IN COLUMN-MAJOR FORMAT
    switch (var->class_type) {
        
        case MAT_C_DOUBLE:
        {
            const double * re = (double *)(complex_data->Re);
            const double * im = (double *)(complex_data->Im);
            
            MatrixXcf mat(num_rows,num_cols);
            
            Complex_t * x = mat.data();
            
            for (int k = 0; k < N; k++) {
                x[k] = std::complex<float>(re[k],im[k]);
            }
            
            output = mat;
            break;
        }
            
        case MAT_C_SINGLE:
        {
            const float * re = (float *)(complex_data->Re);
            const float * im = (float *)(complex_data->Im);
            
            MatrixXcf mat(num_rows,num_cols);
            
            Complex_t * x = mat.data();
            
            for (int k = 0; k < N; k++) {
                x[k] = std::complex<float>(re[k],im[k]);
            }
            
            output = mat;
            break;
        }
            
        
        
        default:
        {
            return false;
        }
        
    }
    
    return true;
  
}

bool MatlabReader::read_real_matrix(const std::string & filename, const std::string & matname,MatrixXf & output) {
    
    mat_t * matfile = Mat_Open(filename.c_str(), MAT_ACC_RDONLY);
    
    if(matfile == NULL) {
        return false;
    }
    
    matvar_t * var = Mat_VarRead(matfile, matname.c_str());
    
    bool worked = read_real(var, output);
    
    Mat_Close(matfile);
    
    return worked;
}

bool MatlabReader::read_complex_matrix(const std::string & filename, const std::string & matname,MatrixXcf & output) {
 
    mat_t * matfile = Mat_Open(filename.c_str(), MAT_ACC_RDONLY);

    if(matfile == NULL) {
        return false;
    }
    
    matvar_t * var = Mat_VarRead(matfile, matname.c_str());

    
    bool worked = read_complex(var, output);
    
    Mat_Close(matfile);
    
    return worked;
}

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
    
    
    matvar_t * frames = Mat_VarGetStructFieldByName(p,"Frames",0);
    
    
    bool worked = read_complex(frames, output);
    
    Mat_Close(matfile);
    
    return worked;
}
