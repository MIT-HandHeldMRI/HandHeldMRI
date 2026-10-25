#!/bin/bash

outputdir="/home/mrilab/64800_MRI/Visualizer/vis/Data/${1}/"
mkdir ${outputdir}

timeout 5s bwrap \
--ro-bind /usr /usr \
--ro-bind /lib64 /lib64 \
--ro-bind /lib /lib \
--ro-bind /home/mrilab/64800_MRI/Visualizer/vis/Scripts /script \
--ro-bind /home/mrilab/64800_MRI/Visualizer/main.cpp /upload/main.cpp \
--ro-bind /home/mrilab/64800_MRI/Visualizer/arduino_stubs.h /upload/arduino_stubs.h \
--ro-bind "/home/mrilab/64800_MRI/Visualizer/vis/Uploads/${1}.cpp" /upload/code.cpp \
--bind ${outputdir} /upload/outputs \
--unshare-all \
--new-session \
--die-with-parent \
--chdir /upload \
/usr/bin/bash /script/sim.sh