#!/bin/bash
virtualenv -p python3 plotting
source plotting/bin/activate
pip install pyqtgraph
pip install PyQt5
pip install zmq
pip install queue
pip install protobuf
