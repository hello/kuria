import numpy as np
import time
import zmq
import novelda_pb2
import scipy.io as io
import sys

def row_to_protobuf(vec,count):
    a = novelda_pb2.RadarFrame()
    a.base_band = True
    a.frame_id = count
    for i in range(vec.shape[0]):
        a.range_bins.append(np.real(vec[i]))
        a.range_bins.append(np.imag(vec[i]))

    return a

def main():
    """main method"""
    matfile = io.loadmat(sys.argv[1])
    baseband = matfile['RecFrames'][0][0][2]

    # Prepare our context and publisher
    context   = zmq.Context()
    publisher = context.socket(zmq.PUB)
    publisher.bind("tcp://127.0.0.1:5563")
    while(True):
        for i in range(baseband.shape[0]):
            print 'publish ', i
            a = row_to_protobuf(baseband[i,:],i)
            s = a.SerializeToString()

#            if i == 0:
#                with open('novelda.bin','w') as f:
#                    f.write(s)
      
            publisher.send(s)
            time.sleep(0.05)

    # We never get here but clean up anyhow
    publisher.close()
    context.term()

if __name__ == "__main__":
    main()
