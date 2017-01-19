#include "matlabwriter.h"
#include <cstdlib>
#include "matio.h"

MatlabWriter * MatlabWriter::_instance = nullptr;

void atexit_handler() {
    MatlabWriter::deinitialize();
}

MatlabWriter::MatlabWriter()
:_is_open(false)
,_mat_file_ptr(nullptr) {
    //register exit handler to be called when program exits (after main)
    std::atexit(atexit_handler);
}

MatlabWriter::~MatlabWriter() {
    
}

MatlabWriter * MatlabWriter::get_instance() {
    
    if (!_instance) {
        _instance = new MatlabWriter();
    }
    
    return _instance;
}

void MatlabWriter::deinitialize() {
    if (_instance) {
        
        if (_instance->_is_open) {
            _instance->close();
        }
        
        delete _instance;
        _instance = nullptr;
    }
}

bool MatlabWriter::open_new_matfile(const std::string & filepath) {
    if (_mat_file_ptr) {
        return false;
    }
    
    mat_t * matfile  = Mat_CreateVer(filepath.c_str(),NULL,MAT_FT_DEFAULT);
    
    if (!matfile) {
        //TODO add error message
        return false;
    }
    
    _mat_file_ptr = matfile;
    
    
    return true;
    
}


bool MatlabWriter::write_real_matrix(const std::string & varname, const MatrixXf & mat) {
    mat_t * matfile = static_cast<mat_t *>(_mat_file_ptr);

    if (!matfile) {
        //TODO add error message
        return false;
    }
    
    size_t dims[2];
    dims[0] = mat.rows();
    dims[1] = mat.cols();
    
    matvar_t * var = Mat_VarCreate(varname.c_str(), MAT_C_SINGLE,MAT_T_SINGLE, 2, dims,(void *)mat.data(), 0);
    
    if (!var) {
        return false;
    }
    
    int ret = Mat_VarWrite(matfile, var, MAT_COMPRESSION_ZLIB);
  
    Mat_VarFree(var);
    
    
    return ret == 0;
    
}

bool MatlabWriter::write_complex_matrix(const std::string & varname, const MatrixXcf & mat) {
    mat_t * matfile = static_cast<mat_t *>(_mat_file_ptr);
    
    if (!matfile) {
        //TODO add error message
        return false;
    }
    
    size_t dims[2];
    dims[0] = mat.rows();
    dims[1] = mat.cols();
    
    const Eigen::MatrixXf real = mat.real();
    const Eigen::MatrixXf imag = mat.imag();

    struct mat_complex_split_t z = {(void *)real.data(),(void *)imag.data()};

    
    matvar_t * var = Mat_VarCreate(varname.c_str(), MAT_C_SINGLE,MAT_T_SINGLE, 2, dims,&z, MAT_F_COMPLEX);
    
    if (!var) {
        return false;
    }
    
    int ret = Mat_VarWrite(matfile, var, MAT_COMPRESSION_ZLIB);
    
    Mat_VarFree(var);
    
    
    return ret == 0;
}

void MatlabWriter::close() {
    mat_t * matfile = static_cast<mat_t *>(_mat_file_ptr);

    if (!matfile) {
        return;
    }
    
    Mat_Close(matfile);
    
    _mat_file_ptr = nullptr;
    
}










