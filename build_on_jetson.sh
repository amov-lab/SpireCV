#!/bin/bash -e

rm -rf build
mkdir build
cd build
cmake .. -DPLATFORM=JETSON
make -j4
sudo make install

