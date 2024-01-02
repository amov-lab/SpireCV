#!/bin/sh

curl -L https://repositories.intel.com/graphics/intel-graphics.key | sudo apt-key add -
sudo apt-add-repository 'deb [arch=amd64] https://repositories.intel.com/graphics/ubuntu focal main'
sudo apt-get update
sudo apt-get install -y -q --no-install-recommends clinfo intel-opencl-icd intel-media-va-driver-non-free

