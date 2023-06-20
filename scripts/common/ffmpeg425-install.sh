#!/bin/sh


sudo apt install -y \
build-essential yasm cmake libtool libc6 libc6-dev unzip wget libfmt-dev \
libnuma1 libnuma-dev libx264-dev libx265-dev libfaac-dev libssl-dev

root_dir=${HOME}"/SpireCV"
if [ ! -d ${root_dir} ]; then
  echo -e "\033[32m[INFO]: ${root_dir} not exist, creating it ... \033[0m"
  mkdir -p ${root_dir}
fi
cd ${root_dir}

git clone https://gitee.com/jario-jin/nv-codec-headers.git
cd nv-codec-headers
git checkout n11.1.5.0
sudo make install
cd ..

wget https://ffmpeg.org/releases/ffmpeg-4.2.5.tar.bz2
tar -xjf ffmpeg-4.2.5.tar.bz2
cd ffmpeg-4.2.5
export PATH=$PATH:/usr/local/cuda/bin
sed -i 's#_30#_75#' configure; sed -i 's#_30#_75#' configure
./configure \
--enable-nonfree \
--enable-gpl \
--enable-shared \
--enable-ffmpeg \
--enable-ffplay \
--enable-ffprobe \
--enable-libx264 \
--enable-libx265 \
--enable-cuda-nvcc \
--enable-nvenc \
--enable-cuda \
--enable-cuvid \
--enable-libnpp \
--extra-libs="-lpthread -lm" \
--extra-cflags=-I/usr/local/cuda/include \
--extra-ldflags=-L/usr/local/cuda/lib64 
make -j8
sudo make install
cd ..

git clone https://gitee.com/jario-jin/ZLMediaKit.git
cd ZLMediaKit
git submodule update --init
mkdir build
cd build
cmake ..
make -j4
cd ..
cd ..

mkdir ZLM
cd ZLM
cp ../ZLMediaKit/release/linux/Debug/MediaServer .
cp ../ZLMediaKit/release/linux/Debug/config.ini .
cd ..

