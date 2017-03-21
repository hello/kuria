#ifndef _PREPROCESSOR_INTERFACE_H_
#define _PREPROCESSOR_INTERFACE_H_

#include "haltija_types.h"
#include <Eigen/Core>
#include <memory>


#define PREPROCESSOR_FLAGS_NOT_READY      (0x00)
#define PREPROCESSOR_FLAGS_FRAME_READY    (0x01)
#define PREPROCESSOR_FLAGS_SEGMENT_READY  (0x02)

class PreprocessorInterface {
public:
    virtual ~PreprocessorInterface() {}
    
    virtual void reset() = 0;
    
    //return flags
    virtual uint32_t add_frame(const BasebandDataFrame_t & input, Eigen::MatrixXcf & filtered_frame, Eigen::MatrixXcf & segment) = 0;

};


typedef std::shared_ptr<PreprocessorInterface> PreprocessorPtr_t;


#endif //_PREPROCESSOR_INTERFACE_H_
