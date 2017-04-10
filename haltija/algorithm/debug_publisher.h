#ifndef _DEBUGPUBLISHER_H_
#define _DEBUGPUBLISHER_H_

#include "debug_publisher_interface.h"

class DebugPublisher {
public:
    static void initialize(DebugPublisherInterface * publisher);
    static void deinitialize();
    
    static DebugPublisherInterface * get_instance();
private:
    static DebugPublisherInterface * _instance;
};



template <class T>
void debug_save(const std::string & my_id, const T & mat) {
    DebugPublisher::get_instance()->publish(my_id,mat);
}

#endif //_DEBUGPUBLISHER_H_

