import novelda_pb2
import sys
import base64
import scipy.io
import numpy as np
def main():

    if len(sys.argv) < 3:
        print 'you need an input and an output file'
        return 

    data = {'range_bins' : [], 'frame_id' : [], 'base_band' : []}

    with open(sys.argv[1]) as f:
        for line in f:
            rf = novelda_pb2.RadarFrame()
            b64_string = line
            b64_string += '=' * (-len(b64_string) % 4)
            my_bytes = base64.b64decode(b64_string)
            rf.ParseFromString(my_bytes)

            v = np.array([f for f in rf.range_bins])
            v2 = v[0::2] + v[1::2]*1j
            
            data['range_bins'].append(v2)

            if rf.HasField('base_band'):
                data['base_band'].append(int(rf.base_band))

            if rf.HasField('frame_id'):
                data['frame_id'].append(rf.frame_id)
            
     
    scipy.io.savemat(sys.argv[2],data)
            
if __name__ == '__main__':
    main()
