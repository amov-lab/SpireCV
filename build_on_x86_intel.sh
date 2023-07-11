#!/bin/bash -e

rm -rf build
mkdir build
cd build 
cmake .. -DPLATFORM=X86_INTEL
make -j4
sudo make install

