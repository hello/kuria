#! /bin/sh

sig=2
echo "Beginning Doraiba Test"
rm data* -v
xterm -e ./build/radar_publisher &
xterm -e ./build/radar_subscriber & 
sleep_time=10
sleep_unit=s
if [ "$#" -eq 1 ]
then
    sleep_time=$1$sleep_unit
elif [ "$#" -eq 2 ]
then
    sleep_time=$1$2
else
    sleep_time=$sleep_time$sleep_unit
fi
echo "running test for:  $sleep_time"
sleep $sleep_time
PUB_PID=$(pidof radar_publisher)
SUB_PID=$(pidof radar_subscriber)
echo "ending subscriber"
echo "ending publisher"
kill -$sig $SUB_PID
kill -$sig $PUB_PID
filename=$(ls data*)
my_path=$(pwd)
echo "sending $my_path/$filename"
send_to="divya-work@192.168.128.243:/Users/divya-work/Documents/Firmware/sati/x4/doraiba_data"
scp $filename $send_to 
echo "Done"
exit 0

