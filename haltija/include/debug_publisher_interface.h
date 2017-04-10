#ifndef _DEBUGPULBISHERINTERFACE_H_
#define _DEBUGPULBISHERINTERFACE_H_

#include <Eigen/Core>

class DebugPublisherInterface {
public:
  virtual ~DebugPublisherInterface() {}

  virtual void publish(const std::string & id, const Eigen::MatrixXcf & mat) = 0;
  virtual void publish(const std::string & id, const Eigen::MatrixXf & mat) = 0;

};

#endif //_DEBUGPULBISHERINTERFACE_H_
