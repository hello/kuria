#ifndef _MATLABWRITER_H_
#define _MATLABWRITER_H_

#include <string>
#include <Eigen/Core>
#include <unordered_map>
#include <vector>

using Eigen::MatrixBase;
using Eigen::MatrixXf;
using Eigen::MatrixXcf;



class CellItem {
public:
    CellItem(const MatrixXcf & m);
    CellItem(const MatrixXf & m);

    enum EMatType_t {
        real,
        complex
    };
    
    const MatrixXcf _c;
    const MatrixXf _f;
    const EMatType_t _e;
    
};

//singleton pattern
class MatlabWriter {
private:
    MatlabWriter();
    ~MatlabWriter();
    
    static MatlabWriter * _instance;
    
    ////////////////
    bool _is_open;
    void * _mat_file_ptr;
    
    typedef std::vector<CellItem> CellArray_t;
    typedef std::unordered_map<std::string,CellArray_t> CellArrayMap_t;
    
    CellArrayMap_t _cellmap;

    
public:
    static MatlabWriter * get_instance();
    static void deinitialize(); //don't call this evar
    
    bool open_new_matfile(const std::string & filepath);
    
    bool write_matrix(const std::string & varname, const MatrixXf & mat);
    bool write_matrix(const std::string & varname, const MatrixXcf & mat);
    bool write_matrix_to_cell_array(const std::string & varname, const MatrixXf & mat);
    bool write_matrix_to_cell_array(const std::string & varname, const MatrixXcf & mat);

    void write_cell_arrays();
    
    void close();

};

static inline void close_matfile() {
    MatlabWriter::get_instance()->close();
}

static inline bool open_new_matfile(const std::string & filepath) {
    return MatlabWriter::get_instance()->open_new_matfile(filepath);
}

template <class T>
bool write_matrix(const std::string & varname,const T & matrix) {
    return MatlabWriter::get_instance()->write_matrix(varname,matrix);
}

template <class T>
bool write_matrix_to_cell_array(const std::string & varname,const T & matrix) {
    return MatlabWriter::get_instance()->write_matrix_to_cell_array(varname,matrix);
}

#endif //_MATLABWRITER_H_
