
# from pyradarlib4 import X4Driver
# from pyradarlib4.apps import socketserver
# # print inspect.getsource(socketserver.main)
    		   
# def main():
#     r = X4Driver()
#     r.set_pps(550)
#     r.set_dacmax(1100)
#     r.set_dacmin(950)
#     socketserver.run_socketserver(r, fps=20, bb=True, timeout=60, port=5555)

# if __name__ == '__main__':
#     main()



"""
This is a basic example on how to open a socket, wait for 1 connection and 
write radar data to that socket.

The socket is opened and waiting for a connection before radar data streaming is
started

Matlab client example for RF data:

    t = tcpclient('127.0.0.1', 5555)
    framelength = t.read(1,'int32')
    h = plot(NaN)
    
    while 1
        % Write a single byte to notify python that we are ready to receive
        t.write(uint8(0))    
    
        frame = t.read(framelength+1,'int32');
        frameN    = frame(1);
        frameData = frame(2:end); 
        
        if ishandle(h)
            set(h,'YData',abs(frameData))
            drawnow
        else
            break
        end
        %plot(frameData)
        disp(frameN)
    
    end
    
    % Note: important to close connection to notify python that 
    % we're done
    t.delete()
    
    
Matlab client example for baseband data:

    t = tcpclient('127.0.0.1', 5555)
    framelength = t.read(1,'int32')
    h = plot(NaN)
    
    while 1
        % Write a single byte to notify python that we are ready to receive
        t.write(uint8(0))    
        
        len = (framelength+1)*2;
        frame = t.read(len,'double');
        frameN    = frame(1);
        frameData = frame(3:end);
        frameDataIQ = zeros(1,length(frameData)/2);
        j=1;
        for i = 1:2:length(frameData)
            j=j+1;
            frameDataIQ(j) = complex(frameData(i), frameData(i+1));
        end
        
        if ishandle(h)
            set(h,'YData',abs(frameDataIQ))
            drawnow
        else
            break
        end
        
        disp(frameN)
    
    end
    
    % Note: important to close connection to notify python that 
    % we're done
    t.delete()    

"""
import zmq
from time import time,sleep
import numpy as np
import novelda_pb2
        
def run_zmqserver(radar, fps=20, bb=False, timeout=60, port=5563):
    """ Run a socketserver instance
    
    The function expects an initialized radar object, with no streaming operation running.
    
    Parameters:
    ------------------
    radar   : An initialized pyradarlib4 object (or something similar such as an X4Driver object)
    fps     : Integer, optional
        The desired FPS
    bb      : boolean, optional
        If true, use baseband (IQ) data. If false, use RF data.
        Note that the radar object has to be properly initialized to use baseband data!
    timeout : integer, optional
        TCP connection timeout in seconds
    port    : integer, optional
        TCP port    
    """
    from Queue import Queue
    
    context = zmq.Context()
    pub_socket = context.socket(zmq.PUB)

    target = 'tcp://127.0.0.1:%s' % port
    print "binding to %s" % target
    pub_socket.bind(target)
        
    q = Queue()
    
    if bb:
        radar.enable_downconversion()
        framelength = len(radar.sample_iq_frame())
        radar.set_iq_frame_buffer(q)
    else:
        radar.disable_downconversion()
        framelength = len(radar.sample_frame())
        radar.set_frame_buffer(q)
        
    radar.radar.set_fps(fps)
    
    print "framelength", framelength
    if bb:
        frame = np.zeros(framelength+1,dtype='complex')
    else:
        frame = np.zeros(framelength+1,dtype='int32')

    n = 0
    timea = time()
    while(1):

        data = q.get()
        frame[1:] = data.data
        frame[0]  = data.number
        
        # Record the initial frame number
        # The frame counter is an 32-bit uint
        # so it could theoretically wrap but with practical
        # frame rates this would take a very, very long time..
        if n == 0:
            frame0 = data.number
        n = n + 1
                   
        try:
            rf = novelda_pb2.RadarFrame()
            rf.frame_id = data.number
            for cn in data.data:
                rf.range_bins.append(cn.real)
                rf.range_bins.append(cn.imag)
            
            print "pub", data.number
            res = pub_socket.send(rf.SerializeToString())
            if res is not None:
                print res
                print "message not published"
                break
            
        except Exception, e:
            # Todo: We should probably find a more graceful method of closing the connection
            print "Error sending data, the client probably closed the connection"
            print e
            break
    
    # End of with-statement, the socket should be closed automagically
    # Stop radar streaming
    radar.radar.set_fps(0)
    
    timeb       = time()
    timelapsed  = timeb - timea
    fps         = n / timelapsed
    framecount  = data.number - frame0 + 1        

    print "---------------------------------------------------------------"
    print "Received %d frames in %f seconds. That is an average of %f FPS." % (n,timelapsed,fps) 
    print "The first frame number was %d, and the last was %d, that gives a firmware framecount of %d" % (frame0,data.number,framecount)
    if framecount == n:
        print "Firmware framecount matches the received number of frames, so no frames seem to have been lost."
    else:
        print "Firmware framecount doesn't match the received number of frames"
    if q.qsize()>0:
        print "There are %d frames left in the frame queue." % q.qsize()
    print "---------------------------------------------------------------"        
    
           
def main():
    from pyradarlib4 import X4Driver
    
    r = X4Driver()

#   robust
#    r.set_iterations(32)
#    r.set_pps(6)
#    r.set_dacmax(1254)
#    r.set_dacmin(800)

#   max sensitivity
    r.set_iterations(64)
    r.set_pps(75)
    r.set_dacmin(948)
    r.set_dacmax(1098)

    try:
        run_zmqserver(r, fps=20, bb=True, timeout=60, port=5563)
    except KeyboardInterrupt, e:
        r.radar.set_fps(0)


if __name__ == '__main__':
    main()
