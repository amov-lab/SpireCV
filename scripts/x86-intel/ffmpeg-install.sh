#!/bin/sh

sudo apt install -y v4l-utils build-essential yasm cmake libtool libc6 libc6-dev unzip wget  libeigen3-dev libfmt-dev libnuma1 libnuma-dev libx264-dev libx265-dev libfaac-dev libssl-dev  v4l-utils
wget https://ffmpeg.org/releases/ffmpeg-4.2.5.tar.bz2
tar -xjf ffmpeg-4.2.5.tar.bz2
cd ffmpeg-4.2.5
./configure \
--arch=x86_64 \
--disable-x86asm \
--enable-vaapi \
--enable-libmfx \
--enable-nonfree \
--enable-shared \
--enable-ffmpeg \
--enable-ffplay \
--enable-ffprobe \
--enable-libx264 \
--enable-libx265 \
--enable-gpl

make -j8
sudo make install
cd ..
sudo rm -r ffmpeg-4.2.5
sudo rm ffmpeg-4.2.5.tar.bz2

