% close all
clear all;
fclose all;
 
global bHeaderFound
global Header

bHeaderFound=0;
Header=[];

% %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% START CONFIGURE

bExtendFormat=1; % 0 - no header data, 1  new header extended format. See XTWAGetFrame.m 
TimeBetweenPlotUpdates=1.0; % time (s) between plot updates
bPlotPhase=0; % 0- amp , 1 - phase, 2 diff-phase

filexml = '.\radar_respiration_rawdata.xml';

% END CONFIGURE
% %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

if exist(filexml, 'file')==0
     errordlg([filexml ' doesnt exist']);
     assert(0, [filexml ' doesnt exist']);
end


FileName = '.\XethruWinAcquisition\Windows\tmpout.dat';
newframe = XTWAGetFrame('init',FileName);
fig = figure(10);


exepath = '.\XethruWinAcquisition\Windows\XethruWinAcquisition.exe';

systemcommand = ['start /high ' exepath ' -e ' filexml];
system(systemcommand);

pause(20); % ensure XethruWinAcquisition is started 

counter = 0;
counter_tresh=10; % start value - adapted to FPS
frame_counter=0;
acc_proc_time=0.0;

newFrame = XTWAGetFrame('frame',FileName,bExtendFormat);

if (~bHeaderFound)
    display('Header not found !');
    pause;
end

lastTimeStamp = 0;
framesDropped = 0;

tic

while (ishandle(fig)  )
    
%     newFrame = XTWAGetFrame('frame',FileName,bExtendFormat);   

    [ newFrame,contentId,timeStamp ] = XTWAGetFrame('frame',FileName,bExtendFormat);
    if newFrame(1) == 0 
          fprintf(1, 'Empty frame\n');
    end
            
    if ((timeStamp - lastTimeStamp) > 1)
       framesDropped = framesDropped + timeStamp - lastTimeStamp - 1;
       fprintf(1, 'Frames dropped: %d (of %d total)\n', framesDropped, timeStamp);
    end
    lastTimeStamp = timeStamp;
    
    proc_time=toc;
    tic
    acc_proc_time=acc_proc_time+proc_time;
    
    frame_counter=frame_counter+1;
    
    if (counter > counter_tresh)
        counter = 0;
       
        FPS_estim=frame_counter/acc_proc_time;
        counter_tresh = max(1,round(FPS_estim*TimeBetweenPlotUpdates));

        plot(newFrame);
        grid on
        ylim([40 60]);
        title([ 'FrameCounter= ' num2str(frame_counter) ' FPS=' num2str(FPS_estim,4)]);

        drawnow;
    end
    counter = counter+1;
end


FPS_estim=frame_counter/acc_proc_time

Nlen=length(newFrame);            

byebye = XTWAGetFrame('exit');   