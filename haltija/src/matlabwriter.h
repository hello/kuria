#ifndef _MATLABWRITER_H_
#define _MATLABWRITER_H_

#include <string>
#include <Eigen/Core>

using Eigen::MatrixXf;
using Eigen::MatrixXcf;

//singleton pattern
class MatlabWriter {
private:
    MatlabWriter();
    ~MatlabWriter();
    
    static MatlabWriter * _instance;
    
    ////////////////
    bool _is_open;
    void * _mat_file_ptr;
    
public:
    static MatlabWriter * get_instance();
    static void deinitialize(); //don't call this evar
    
    bool open_new_matfile(const std::string & filepath);
    
    bool write_new_matrix(const std::string & varname, const MatrixXf & mat);
    bool write_new_matrix(const std::string & varname, const MatrixXcf & mat);
    
    void close();

};


#endif //_MATLABWRITER_H_
