import numpy as np
import time
import zmq
import radar_messages_pb2
import sys

target = "tcp://127.0.0.1:6543"

def row_to_protobuf(vec,count):
    a = radar_messages_pb2.FeatureVector()
    a.id = "foobars"
    a.sequence_number = count
    for i in range(vec.shape[0]):
        a.floatfeats.append(vec[i])

    return a

def main():
    """main method"""
    # Prepare our context and publisher
    context   = zmq.Context()
    publisher = context.socket(zmq.PUB)
    publisher.bind(target)
    while(True):
        for i in range(100000):
            print 'publish ', i
            y = np.array([np.sin(i / 20.0)])
            a = row_to_protobuf(y,i)
            s = a.SerializeToString()

            publisher.send_multipart(["PLOT",s])
            time.sleep(0.05)

    # We never get here but clean up anyhow
    publisher.close()
    context.term()

if __name__ == "__main__":
    main()
