#!/bin/sh

sudo apt install -y libgstreamer1.0-dev libgstreamer-plugins-base1.0-dev
sudo apt install -y libgstreamer-plugins-bad1.0-dev gstreamer1.0-plugins-base
sudo apt install -y gstreamer1.0-plugins-good gstreamer1.0-plugins-bad 
sudo apt install -y gstreamer1.0-plugins-ugly gstreamer1.0-libav gstreamer1.0-doc
sudo apt install -y gstreamer1.0-tools gstreamer1.0-x gstreamer1.0-alsa
sudo apt install -y gstreamer1.0-gl gstreamer1.0-gtk3 gstreamer1.0-qt5 
sudo apt install -y gstreamer1.0-pulseaudio
sudo apt install -y gtk-doc-tools

git clone https://gitee.com/jario-jin/gst-rtsp-server-b18.git
cd gst-rtsp-server-b18
./autogen.sh
make
sudo make install
cd ..
sudo rm -r gst-rtsp-server-b18

