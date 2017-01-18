#ifndef _MATLABHELPER_H_
#define _MATLABHELPER_H_

#include <Eigen/Dense>
#include <string>


class MatlabHelper {
public:
    static bool read_baseband_from_file_v1(const std::string & filename, Eigen::MatrixXcf & output);
    
};

#endif //_MATLABHELPER_H_
