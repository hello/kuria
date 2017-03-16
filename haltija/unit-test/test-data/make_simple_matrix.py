import simple_matrix_pb2
import struct
s = simple_matrix_pb2.SimpleMatrix()
s.id = "name"
s.device_id = "device_id"
s.num_cols = 2
s.data_type=5
s.payload.append(struct.pack('dd',1.0,3.14159))
f = open('simplematrix.bin','w')
f.write(s.SerializeToString())
f.close()
