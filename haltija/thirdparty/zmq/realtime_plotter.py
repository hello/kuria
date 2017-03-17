#!/usr/bin/python
import pyqtgraph
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
#sys.path.append('.')


np.set_printoptions(precision=3, suppress=True, threshold=np.nan)

#def signal_handler(signal, frame):
 #       print('You pressed Ctrl+C!')
 #       g_kill = True
 #       sys.exit(0)



plot_samples = 430
num_feats = 16
plot_yrange = (-6000, 10000)
plot_num_signal = num_feats + 1

g_kill = False
g_PlotQueue = Queue()

global g_graphicsitems
global g_p6
global g_segdata
global g_plotdata
global g_curves

g_curves = []
g_plotdata = []
g_segdata = [0 for j in range(plot_samples)]
g_graphicsitems = []
g_p6 = None

    
def CreatePlotCurves(p6):
     p6.setRange(xRange=(0, plot_samples-1), yRange=plot_yrange)

     curves = []
     for i in range(plot_num_signal):
        curves.append(p6.plot(pen=(i, plot_num_signal)))
        
     return curves
     
def Refresh(p6, curves):
    for c in curves:
        p6.removeItem(c)
        p6.addItem(c)
        
def updatePlot():
    global g_graphicsitems
    global g_p6
    global g_segdata
    global g_plotdata
    global g_curves
    
    try:
        while True:
            block = g_PlotQueue.get(False)
            if block.mytype_ == 'audiofeatures':
                vec = block.data_.idata
                idx = block.data_.time1 % plot_samples
                for j in range(len(vec)):
                    g_plotdata[j][idx]= (vec[j]);
                
                
                #reset plot signals
                if idx == plot_samples - 1:
                    g_plotdata = []
                    g_segdata = [0 for j in range(plot_samples)]

                    #fill in empy buffers
                    for j in range(num_feats):
                        g_plotdata.append([0 for j in range(plot_samples)])

                    g_plotdata.append(g_segdata)

        #remove text
                    for gitem in g_graphicsitems:
                        g_p6.removeItem(gitem)
             
                    g_graphicsitems = []
            
                    Refresh(g_p6, g_curves)
            
                else:
                    #update plot
                    for j in range(plot_num_signal):
                        g_curves[j].setData(g_plotdata[j])
            
            if block.mytype_ == 'segdata':
                s1 = block.data_[0]
                s2 = block.data_[1]
                g_segdata[s1[0]] = s1[1]
                g_segdata[s2[0]] = s2[1]
                segtype = block.data_[2]
                
                text = pg.TextItem(segtype, anchor=(0, 0))
                text.setPos(s2[0], 0)
                g_graphicsitems.append(text)
                g_p6.addItem(text)

            
            if block.mytype_ == 'block':
                vec = block.data_.idata
                vec2 = []
                for i in vec:
                    vec2.append(i)
               
                g_curves[0].setData(vec2)
    
    
    except Empty:
        foo = 3
    except Exception:
        raise

def subscribe_messages(target):
    # Prepare our context and publisher
    context    = zmq.Context()
    subscriber = context.socket(zmq.SUB)
    subscriber.connect(target)
    subscriber.setsockopt(zmq.SUBSCRIBE,b"PLOT")

    print (target)
    while not g_kill:
        # Read envelope with address
        try:
             envelope,message = subscriber.recv_multipart()
             vec = radar_messages_pb2.FeatureVector()
             vec.ParseFromString(message)

             x = [f for f in vec.floatfeats]
             print(x)
             g_PlotQueue.put(x)
                  
        except IOError:
             pass
          
    # We never get here but clean up anyhow
    subscriber.close()
    context.term()


def test():
    subscribe_messages(sys.argv[1])
     
def main_plotter():
    argc = len(sys.argv)
    if argc > 1:
        plot_target = sys.argv[1]
        
    if argc > 2:
        plot_yrange = (-int(sys.argv[2]), int(sys.argv[2]))
        
    if argc > 3:
        plot_samples = int(sys.argv[3])
    
    #signal.signal(signal.SIGINT, signal_handler)
    paud = pyaudio.PyAudio()
    app = QtGui.QApplication([])

    plotTimer = QtCore.QTimer()
    plotTimer.timeout.connect(updatePlot)
    plotTimer.start(10)
    


    win = pg.GraphicsWindow(title="Basic plotting examples")
    win.resize(640,480)
    win.setWindowTitle('sound!')
    pg.setConfigOptions(antialias=True)

    g_p6 = win.addPlot(title=plot_target)
   
    g_curves = CreatePlotCurves(g_p6)

    for j in range(num_feats):
        g_plotdata.append([0 for j in range(plot_samples)])
    g_plotdata.append(g_segdata)

                
    t = threading.Thread(target=subscribe_messages, args = (target,))
    t.start()


    if (sys.flags.interactive != 1) or not hasattr(QtCore, 'PYQT_VERSION'):
        #this blocks
        QtGui.QApplication.instance().exec_()
    
    g_kill = True

## Start Qt event loop unless running in interactive mode or using pyside.
if __name__ == '__main__':
    test()
