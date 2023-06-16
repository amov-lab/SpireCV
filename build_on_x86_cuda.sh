#!/bin/bash -e

rm -rf build
mkdir build
cd build 
cmake .. -DPLATFORM=X86_CUDA
make -j4
sudo make install

