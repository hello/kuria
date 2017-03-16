#ifndef _MATLABHELPER_H_
#define _MATLABHELPER_H_

#include <Eigen/Dense>
#include <string>



namespace MatlabReader {
    bool read_real_matrix(const std::string & filename, const std::string & matname,Eigen::MatrixXf & output);

    bool read_complex_matrix(const std::string & filename, const std::string & matname,Eigen::MatrixXcf & output);
    
    bool read_baseband_from_file_v1(const std::string & filename, Eigen::MatrixXcf & output);
};

#endif //_MATLABHELPER_H_
