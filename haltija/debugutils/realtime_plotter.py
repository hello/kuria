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

server_address = "tcp://127.0.0.1:5564"
signal_target = "maxvarresp"
histogram_target = "modes"
np.set_printoptions(precision=3, suppress=True, threshold=np.nan)

plot_samples = 200
num_feats = 2
max_num_histogram = 5

g_kill = False
g_PlotQueue = Queue()

global g_graphicsitems
global g_p6
global g_p7
global g_p7_curves
global g_histogram_data
global g_plotdata
global g_curves
global g_yrangemax
global g_prev_histogram_index

g_curves = []
g_graphicsitems = []
g_p6 = None
g_p7 = None
g_p7_curves = None
g_histogram_data = []
g_yrangemax = 1e2
g_yrangemin = 2e0
g_prev_histogram_index = 0

plot_yrange = (-g_yrangemax, g_yrangemax)

def CreatePlotCurves(p6):
     p6.setRange(xRange=(0, plot_samples-1), yRange=plot_yrange)

     curves = []
     for i in range(num_feats):
        curves.append(p6.plot(pen=(i, num_feats)))
        
     return curves
     
def CreateHistogramCurves(p7):
     p7.setYRange(0,0.5)

     curves = []
     for i in range(max_num_histogram):
        curves.append(p7.plot(stepMode=True,fillLevel=0,brush=(0,0,255,150),pen=(i, max_num_histogram)))

     return curves

     
def plot_signal(index,vec,plotdata,plt,curves,x_range,y_range,y_range_min):
    plotdata[0].append(vec[0]);
    plotdata[1].append(vec[1]);

    the_max = max([max(plotdata[0]),max(plotdata[1])])
    the_min = min([min(plotdata[0]),min(plotdata[1])])
    yrangemax = max((the_max,-the_min,y_range_min))
    plot_yrange = (-yrangemax,yrangemax)
    if plt != None:
        plt.setRange(xRange=x_range, yRange=plot_yrange)

    for j in range(len(curves)):
        curves[j].setData(list(plotdata[j]))

def plot_histogram(index,prev_index,data,vec,curves):
     x = np.array(range(len(vec) + 1));
     y = np.array(vec)

     if index != prev_index:
          del data[:]
          for curve in curves:
               curve.setData([0,0],[0])

     data.append(y)

     for i,d in enumerate(data):
          if i >= len(curves):
               break;

          curves[i].setData(x,d)
          
def update_plot():
    global g_graphicsitems
    global g_p6
    global g_p7
    global g_plotdata
    global g_curves
    global g_yrangemax
    global g_prev_histogram_index
    global g_histogram_data
    global g_p7_curves
    
    try:
        while True:
            message_id,index,vec = g_PlotQueue.get(False)

            if (message_id == signal_target):
                 plot_signal(index,vec,g_plotdata,g_p6,g_curves,(0, plot_samples - 1),None,g_yrangemin)
            if (message_id == histogram_target):
                 plot_histogram(index,g_prev_histogram_index,g_histogram_data,vec,g_p7_curves)
                 g_prev_histogram_index = index
          
    except Empty:
        pass
    except Exception:
        raise

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
             message_id = vec.id
             x = [f for f in vec.floatfeats]
             #print((index,x))
             #print(message_id)
             g_PlotQueue.put((message_id,index,x))
             if (index % 100 == 0):
                 print('got message %d' % (index))     

        except IOError:
             pass
          
    # We never get here but clean up anyhow
    subscriber.close()
    context.term()
     
def main_plotter():
    global g_plotdata
    global g_curves
    global g_kill
    global g_p6
    global g_p7
    global g_p7_curves
    
    argc = len(sys.argv)
    #signal.signal(signal.SIGINT, signal_handler)
    app = QtGui.QApplication([])

    plotTimer = QtCore.QTimer()
    plotTimer.timeout.connect(update_plot)
    plotTimer.start(10)
    

    
    win = pg.GraphicsWindow(title="Basic plotting examples")
    win.resize(640,480)
    win.setWindowTitle('plotter')
    pg.setConfigOptions(antialias=True)

    g_p6 = win.addPlot(title="baseband signal")
    g_p7 = win.addPlot(title="bins of interest")

    g_curves = CreatePlotCurves(g_p6)
    g_p7_curves = CreateHistogramCurves(g_p7)
    
    g_plotdata = []
    g_histogram_data = []
    
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
