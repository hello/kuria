
#include "network/zmq_subscriber.h"
#include "network/noveldaprotobuf.h"
#include "algorithm/novelda_radar_subscriber.h"
#include "haltija_types.h"


int main(int argc, char * argv[]) {
    
    ZmqSubscriber<NoveldaProtobuf,NoveldaData_t,NoveldaRadarSubscriber> zmq_subscriber(100000,"tcp://localhost:5563");
    
    zmq_subscriber.add_subscriber("foobars", new NoveldaRadarSubscriber());
    
    zmq_subscriber.run();
    
    return 0;
}
