#!/bin/sh

sudo apt-get install -y libmfx1 libmfx-tools libva-dev libmfx-dev intel-media-va-driver-non-free vainfo va-driver-all vdpau-va-driver
echo "export LIBVA_DRIVER_NAME=i965" >> ~/.bashrc
export LIBVA_DRIVER_NAME=i965

