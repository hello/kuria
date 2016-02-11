function [ frame,contentId,timeStamp ] = XTWAGetFrame( command, filename ,bExtendFormat)

global bHeaderFound
global Header

%XTWASGETFRAME Summary of this function goes here
%   Detailed explanation goes here

% bExtendFormat=1 Header info - 
%             frame.append(1);
%             frame.append(contentId=3);
%             frame.append(info);
%             frame.append(11);
% 
%             frame.append(pmsg->cfgPulsesPerStep);
%             frame.append(pmsg->cfgIterations);
%             frame.append(pmsg->cfgDACStep);
%             frame.append(pmsg->cfgDACAuto);
%             frame.append(pmsg->cfgMClkDiv);
%             frame.append(pmsg->samplesPerSecond);
%             frame.append(pmsg->cfgSampleDelay);
%             frame.append(pmsg->carrierFreqFromPG[pmsg->cfgPGSelect]);
%             frame.append(pmsg->cfgPGSelect);
%             frame.append(pmsg->cfgFrameStitch);
%             frame.append(pmsg->cfgFPS);


    persistent fileExchange mmExchange
    
    contentId = 0;
    timeStamp = 0;

    if (nargin==3) 
        if (bExtendFormat==0)
            bHeaderFound=1;  % headerfound test not used
            Header=[];
        end
    end
    
    if strcmp(command, 'init') 
        if ~isempty(fileExchange) && fileExchange>0
            fclose(fileExchange);
        end   
        delete(filename);
        if exist(filename, 'file' )~=0
            errordlg(['unable to delete ' filename]) ;
            assert(false);
        end
        frame = [];
        return;
    end
    
    if strcmp(command, 'exit') && ~isempty(fileExchange)
        % TODO: Tell server to exit.
        mmExchange.Data(1) = -1;
        fclose(fileExchange);
        clear fileExchange mmExchange;
        frame = [];
        return;
    end

    while isempty(fileExchange) || fileExchange < 0
        [fileExchange, msg] = fopen(filename, 'r+b');
        if fileExchange < 0
            fprintf('Waiting for XethruWinAcquisition exchange file...\n');
            pause(.25);
        end
    end
    
    mmExchange = memmapfile(filename, 'Writable', true, 'Format', 'single');
        
    while length(mmExchange.Data) == 0 || mmExchange.Data(1) == 0
        %fprintf('Waiting for XethruWinAcquisition data...\n');
         pause(.01);
    end

    % The first byte now contains the length of the message.
    data = mmExchange.Data;
    datalength = min([data(1) length(data)-1]);
    frame = mmExchange.Data(2:datalength+1);

    if (bExtendFormat)            
        contentId = data(3);
        timeStamp = data(4);
    end
    
    if (nargin==3)
        if (bHeaderFound && bExtendFormat)            
            frame=frame(5:end);
        end
    end
    
    frame = frame';

    if (datalength == 15) % radar settings header
        if (frame(1)==1 && frame(2)==3 && frame(4)==11)
            
            bHeaderFound=1;
            Header=[];
             
            Header.cfgPulsesPerStep=frame(5);
            Header.cfgIterations=frame(6);
            Header.cfgDACStep=frame(7);
            Header.cfgDACAuto=frame(8);
            Header.cfgMClkDiv=frame(9);
            Header.samplesPerSecond=frame(10);
            Header.cfgSampleDelay=frame(11);
            Header.carrierFreqFromPG=frame(12);
            Header.cfgPGSelect=frame(13);
            Header.cfgFrameStitch=frame(14);
            Header.cfgFPS=frame(15);
            
        end
    end
    
    % Signal to XethruWinAcquisition that the data has been read.
    mmExchange.Data(1) = 0;
    
end

