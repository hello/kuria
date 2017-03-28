use zmqclient.c to eavesdrop for all broadcasts with a zmq publisher
to compile: clang zmqclient.c -o zmqclient -lzmq

To use the realtime plotter (receives radar messages' feature vector protobuf message)
you will probably need to run a python virtualenv.  Once you have pip and virtualenv, you can rung
the "install_pyqt.sh" script.  Then, "source plotting/bin/activate" if you haven't already (you'll see your prompt begin with (plotting).  Then, python ./realtime_plotter.py.  Tada.

To broadcast a matfile of radar data (Novelda's .mat format) use baseband_publisher.py
syntax is "python ./baseband_publisher.py some_mat_file.mat" where the baseband is in the "RecFrames" object in the matlab file.  You can then run the haltijanetworkrunner and process that data!




