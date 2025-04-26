#!/bin/bash

SHELL_FOLDER=$(cd "$(dirname "$0")";pwd)  
cd $(dirname $SHELL_FOLDER) 
. devel/setup.bash

sudo chmod 777 /dev/ttyUSB0   # read and write permission
sudo sh -c 'echo 1 > /sys/bus/usb-serial/devices/ttyUSB0/latency_timer'  # set latency timer to 1ms, provides better performance
# roscore
rosrun sri_force_sensor_driver force_sensor_node