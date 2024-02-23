#!/bin/sh

sudo apt-get install -y libmfx1 libmfx-tools libva-dev libmfx-dev intel-media-va-driver-non-free vainfo
echo "export LIBVA_DRIVER_NAME=iHD" >> ~/.bashrc
export LIBVA_DRIVER_NAME=iHD

