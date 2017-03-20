#!/usr/bin/python
import pyqtgraph as pg
from pyqtgraph.Qt import QtGui, QtCore
import numpy as np
import sys
import threading
import signal
import struct
from queue import Queue
from queue import Empty
import copy
import radar_messages_pb2
import zmq
import collections

server_address = "tcp://127.0.0.1:6543"

np.set_printoptions(precision=3, suppress=True, threshold=np.nan)

plot_samples = 100
num_feats = 1
plot_yrange = (-1, 1)

g_kill = False
g_PlotQueue = Queue()

global g_graphicsitems
global g_p6
global g_plotdata
global g_curves

g_curves = []
g_graphicsitems = []
g_p6 = None

    
def CreatePlotCurves(p6):
     p6.setRange(xRange=(0, plot_samples-1), yRange=plot_yrange)

     curves = []
     for i in range(num_feats):
        curves.append(p6.plot(pen=(i, num_feats)))
        
     return curves
     
def Refresh(p6, curves):
    for c in curves:
        p6.removeItem(c)
        p6.addItem(c)
        
def update_plot():
    global g_graphicsitems
    global g_p6
    global g_plotdata
    global g_curves

    
    try:
        while True:
            index,vec = g_PlotQueue.get(False)            
            g_plotdata[0].append(vec[0]);
            for j in range(len(g_curves)):
                g_curves[j].setData(list(g_plotdata[j]))

    except Empty:
        pass
    except Exception:
        raise

    #Refresh(g_p6, g_curves)

def subscribe_messages(publisher_url):
    global g_kill
    # Prepare our context and publisher
    context    = zmq.Context()
    subscriber = context.socket(zmq.SUB)
    subscriber.connect(publisher_url)
    subscriber.setsockopt(zmq.SUBSCRIBE,b"PLOT")

    while not g_kill:
        # Read envelope with address
        try:
             envelope,message = subscriber.recv_multipart()
             vec = radar_messages_pb2.FeatureVector()
             vec.ParseFromString(message)
             index = vec.sequence_number
             x = [f for f in vec.floatfeats]
             print((index,x))
             g_PlotQueue.put((index,x))
                  
        except IOError:
             pass
          
    # We never get here but clean up anyhow
    subscriber.close()
    context.term()
     
def main_plotter():
    global g_plotdata
    global g_curves
    global g_kill
    
    argc = len(sys.argv)
    #signal.signal(signal.SIGINT, signal_handler)
    app = QtGui.QApplication([])

    plotTimer = QtCore.QTimer()
    plotTimer.timeout.connect(update_plot)
    plotTimer.start(10)
    

    
    win = pg.GraphicsWindow(title="Basic plotting examples")
    win.resize(640,480)
    win.setWindowTitle('oy vey!')
    pg.setConfigOptions(antialias=True)

    g_p6 = win.addPlot(title="my title")

    g_p6.setRange(xRange=(0, plot_samples-1), yRange=plot_yrange)
        
    g_curves = CreatePlotCurves(g_p6)
    
    g_plotdata = []
    for i in range(num_feats):
         g_plotdata.append(collections.deque(maxlen=plot_samples))

    #zmq message subscriber
    t = threading.Thread(target=subscribe_messages, args = (server_address,))
    t.start()


    if (sys.flags.interactive != 1) or not hasattr(QtCore, 'PYQT_VERSION'):
        #this blocks
        QtGui.QApplication.instance().exec_()
    
    g_kill = True

## Start Qt event loop unless running in interactive mode or using pyside.
if __name__ == '__main__':
    main_plotter()
