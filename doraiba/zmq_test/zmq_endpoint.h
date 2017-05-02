#ifndef __ZMQ_ENDPOINT_H__
#define __ZMQ_ENDPOINT_H__

#define USE_IPC 1

#if USE_IPC 
#define ZMQ_ENDPOINT "ipc:///home/pi/mylocal.ipc"
//#define ZMQ_ENDPOINT "ipc://~/mylocal.ipc"
#else
#define ZMQ_ENDPOINT "tcp://localhost:5556"
#endif


#endif
