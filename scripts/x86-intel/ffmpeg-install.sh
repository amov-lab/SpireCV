#!/bin/sh

sudo apt install -y v4l-utils
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

