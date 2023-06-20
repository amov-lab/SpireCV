#!/bin/sh


wget https://download.amovlab.com/model/deps/opencv-4.7.0.zip
wget https://download.amovlab.com/model/deps/opencv_contrib-4.7.0.zip
wget https://download.amovlab.com/model/deps/opencv_cache_x86-4.7.0.zip


package_dir="."
mkdir ~/opencv_build


if [ ! -d ""$package_dir"" ];then
  echo "\033[31m[ERROR]: $package_dir not exist!: \033[0m"
  exit 1
fi

# sudo add-apt-repository "deb http://security.ubuntu.com/ubuntu xenial-security main"
# sudo add-apt-repository "deb http://mirrors.tuna.tsinghua.edu.cn/ubuntu-ports/ xenial main multiverse restricted universe"
sudo apt update
sudo apt install -y build-essential
sudo apt install -y cmake git libgtk2.0-dev pkg-config libavcodec-dev libavformat-dev libswscale-dev
sudo apt install -y libjasper1 libjasper-dev
sudo apt install -y python3-dev python3-numpy libtbb2 libtbb-dev libjpeg-dev libpng-dev libtiff-dev
sudo apt install -y libdc1394-22-dev


echo "\033[32m[INFO]:\033[0m unzip opencv-4.7.0.zip ..."
unzip  -q -o $package_dir/opencv-4.7.0.zip -d ~/opencv_build

echo "\033[32m[INFO]:\033[0m unzip opencv_contrib-4.7.0.zip ..."
unzip  -q -o $package_dir/opencv_contrib-4.7.0.zip -d ~/opencv_build

echo "\033[32m[INFO]:\033[0m unzip opencv_cache_x86-4.7.0.zip ..."
unzip  -q -o $package_dir/opencv_cache_x86-4.7.0.zip -d ~/opencv_build


sudo rm opencv-4.7.0.zip
sudo rm opencv_contrib-4.7.0.zip
sudo rm opencv_cache_x86-4.7.0.zip

cd ~/opencv_build/opencv-4.7.0
mkdir .cache

cp -r ~/opencv_build/opencv_cache_x86-4.7.0/* ~/opencv_build/opencv-4.7.0/.cache/

mkdir build
cd build

cmake -D CMAKE_BUILD_TYPE=Release \
      -D WITH_CUDA=OFF \
      -D OPENCV_ENABLE_NONFREE=ON \
      -D CMAKE_INSTALL_PREFIX=/usr/local \
      -D OPENCV_EXTRA_MODULES_PATH=../../opencv_contrib-4.7.0/modules ..

make -j2
sudo make install

cd
sudo rm -r ~/opencv_build
