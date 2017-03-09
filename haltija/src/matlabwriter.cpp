#include "matlabwriter.h"
#include <cstdlib>
#include "matio.h"

MatlabWriter * MatlabWriter::_instance = nullptr;

void atexit_handler() {
    MatlabWriter::deinitialize();
}

CellItem::CellItem(const MatrixXcf & m)
    : _c(m), _f(MatrixXf()),_e(complex) {
    
}

CellItem::CellItem(const MatrixXf & m)
    : _c(MatrixXcf()), _f(m),_e(real) {

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
    _is_open = true;
    _mat_file_ptr = matfile;
    
    
    return true;
    
}



bool MatlabWriter::write_matrix(const std::string & varname, const MatrixXf & mat) {
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

bool MatlabWriter::write_matrix(const std::string & varname, const MatrixXcf & mat) {
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

bool MatlabWriter::write_matrix_to_cell_array(const std::string & varname, const MatrixXcf & mat) {
    if (_cellmap.find(varname) == _cellmap.end()) {
        _cellmap[varname] = CellArray_t();
        _cellmap[varname].reserve(10000);
    }
    
    _cellmap[varname].push_back(CellItem(mat));

    return true;
}

bool MatlabWriter::write_matrix_to_cell_array(const std::string & varname, const MatrixXf & mat) {
    
    
    if (_cellmap.find(varname) == _cellmap.end()) {
        _cellmap[varname] = CellArray_t();
        _cellmap[varname].reserve(10000);
    }
    
    _cellmap[varname].push_back(CellItem(mat));
        
    return true;
}

void MatlabWriter::write_cell_arrays() {
    mat_t * matfile = static_cast<mat_t *>(_mat_file_ptr);

    for (auto arr_it = _cellmap.begin(); arr_it != _cellmap.end(); arr_it++) {
        const char * key = (*arr_it).first.c_str();
        const CellArray_t & arr = (*arr_it).second;
        
        size_t cell_dims[2] = {arr.size(),1};
        
        matvar_t * cell_array = Mat_VarCreate(key,MAT_C_CELL,MAT_T_CELL,2,cell_dims,NULL,0);
        int index = 0;
        for (auto it = arr.begin(); it != arr.end(); it++) {

            switch ((*it)._e) {
                case CellItem::complex:
                {
                    size_t dims[2] = {(size_t)(*it)._c.rows(),(size_t)(*it)._c.cols()};
                    
                    const Eigen::MatrixXf real = (*it)._c.real();
                    const Eigen::MatrixXf imag = (*it)._c.imag();
                    
                    struct mat_complex_split_t z = {(void *)real.data(),(void *)imag.data()};
                    
                    matvar_t * cell_element = Mat_VarCreate(NULL, MAT_C_SINGLE,MAT_T_SINGLE, 2, dims,&z, MAT_F_COMPLEX);
                    
                    Mat_VarSetCell(cell_array,index++,cell_element);
                    break;
                }
                    
                case CellItem::real:
                {
                    size_t dims[2] = {(size_t)(*it)._f.rows(),(size_t)(*it)._f.cols()};
                    matvar_t * cell_element = Mat_VarCreate(NULL,MAT_C_SINGLE,MAT_T_SINGLE,2,dims,(void *)(*it)._f.data(),0);
                    Mat_VarSetCell(cell_array,index++,cell_element);
                    break;
                }
                    
                default:
                {
                    break;
                }
            }
            
        }
        
        Mat_VarWrite(matfile,cell_array,MAT_COMPRESSION_NONE);

        Mat_VarFree(cell_array);

    }
    
}

void MatlabWriter::close() {
    mat_t * matfile = static_cast<mat_t *>(_mat_file_ptr);

    if (!matfile) {
        return;
    }
    
    write_cell_arrays();
    
    Mat_Close(matfile);
    
    _mat_file_ptr = nullptr;
    
}










