#!/bin/sh


current_dir=$(pwd)
cd ${HOME}/SpireCV
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

cd ${current_dir}

