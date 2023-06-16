#!/bin/sh

echo -e "\033[32m[INFO]:\033[0m Please enter the folder path of the installation package: "
# package_dir="/home/jario/Downloads/nv"

wget https://download.amovlab.com/model/install/x86-nvidia/cuda-repo-ubuntu2004-11-6-local_11.6.2-510.47.03-1_amd64.deb
wget https://download.amovlab.com/model/install/x86-nvidia/cudnn-linux-x86_64-8.4.1.50_cuda11.6-archive.tar.xz
wget https://download.amovlab.com/model/install/x86-nvidia/nv-tensorrt-repo-ubuntu2004-cuda11.6-trt8.4.0.6-ea-20220212_1-1_amd64.deb
wget https://download.amovlab.com/model/install/x86-nvidia/cuda-ubuntu2004.pin

package_dir="."

cuda_fn=$package_dir"/cuda-repo-ubuntu2004-11-6-local_11.6.2-510.47.03-1_amd64.deb"
cudnn_fn=$package_dir"/cudnn-linux-x86_64-8.4.1.50_cuda11.6-archive.tar.xz"
tensorrt_fn=$package_dir"/nv-tensorrt-repo-ubuntu2004-cuda11.6-trt8.4.0.6-ea-20220212_1-1_amd64.deb"
tmp_dir="/tmp"

# https://developer.nvidia.com/compute/machine-learning/tensorrt/secure/8.4.0/local_repos/nv-tensorrt-repo-ubuntu2004-cuda11.6-trt8.4.0.6-ea-20220212_1-1_amd64.deb

echo -e "\033[32m[INFO]: CUDA_PKG: \033[0m"$cuda_fn
echo -e "\033[32m[INFO]: CUDNN_PKG: \033[0m"$cudnn_fn
echo -e "\033[32m[INFO]: TENSORRT_PKG: \033[0m"$tensorrt_fn

# 所有文件都存在时，才会继续执行脚本
if [ ! -f "$cuda_fn" ]; then
  echo -e "\033[31m[ERROR]: CUDA_PKG not exist!: \033[0m"
  exit 1
fi

if [ ! -f "$cudnn_fn" ]; then
  echo -e "\033[31m[ERROR]: CUDNN_PKG not exist!: \033[0m"
  exit 1
fi

if [ ! -f "$tensorrt_fn" ]; then
  echo -e "\033[31m[ERROR]: TENSORRT_PKG not exist!: \033[0m"
  exit 1
fi

# 删除显卡驱动
# sudo apt-get remove nvidia-*

# 安装显卡驱动
# echo -e "\033[32m[INFO]: Nvidia Driver installing ...\033[0m"
# sudo add-apt-repository ppa:graphics-drivers/ppa
# ubuntu-drivers devices
# sudo ubuntu-drivers autoinstall
# sudo apt-get install nvidia-driver-465

# 删除已安装CUDA
# --purge选项会将配置文件、数据库等删除
# sudo apt-get autoremove --purge cuda
# sudo apt-get purge nvidia-*
# 查看安装了哪些cuda相关的库，可以用以下指令
# sudo dpkg -l |grep cuda
# sudo dpkg -P cuda-repo-ubuntu1804-10-2-local-10.2.89-440.33.01
# sudo dpkg -P cuda-repo-ubuntu1804-11-1-local
# sudo dpkg -P nv-tensorrt-repo-ubuntu1804-cuda10.2-trt8.0.1.6-ga-20210626
# 这个key值是官网文档查到的，当然通过apt-key list也能查看
# sudo apt-key list
# sudo apt-key del 7fa2af80

# 安装CUDA
echo -e "\033[32m[INFO]: CUDA installing ...\033[0m"
# wget https://developer.download.nvidia.com/compute/cuda/repos/ubuntu1804/x86_64/cuda-ubuntu1804.pin
sudo cp $package_dir/cuda-ubuntu2004.pin /etc/apt/preferences.d/cuda-repository-pin-600
sudo dpkg -i $cuda_fn
sudo apt-key add /var/cuda-repo-ubuntu2004-11-6-local/7fa2af80.pub
sudo apt-get update
sudo apt-get -y install cuda

# 安装CUDNN
echo -e "\033[32m[INFO]: CUDNN installing ...\033[0m"
tar -xvf $cudnn_fn -C $tmp_dir
sudo cp $tmp_dir/cudnn-linux-x86_64-8.4.1.50_cuda11.6-archive/include/cudnn* /usr/local/cuda/include/
sudo cp $tmp_dir/cudnn-linux-x86_64-8.4.1.50_cuda11.6-archive/lib/libcudnn* /usr/local/cuda/lib64/
sudo chmod a+r /usr/local/cuda/include/cudnn* /usr/local/cuda/lib64/libcudnn*

sudo ln -sf /usr/local/cuda/targets/x86_64-linux/lib/libcudnn_cnn_infer.so.8.4.1 /usr/local/cuda/targets/x86_64-linux/lib/libcudnn_cnn_infer.so.8
sudo ln -sf /usr/local/cuda/targets/x86_64-linux/lib/libcudnn.so.8.4.1 /usr/local/cuda/targets/x86_64-linux/lib/libcudnn.so.8
sudo ln -sf /usr/local/cuda/targets/x86_64-linux/lib/libcudnn_ops_train.so.8.4.1 /usr/local/cuda/targets/x86_64-linux/lib/libcudnn_ops_train.so.8
sudo ln -sf /usr/local/cuda/targets/x86_64-linux/lib/libcudnn_adv_train.so.8.4.1 /usr/local/cuda/targets/x86_64-linux/lib/libcudnn_adv_train.so.8
sudo ln -sf /usr/local/cuda/targets/x86_64-linux/lib/libcudnn_ops_infer.so.8.4.1 /usr/local/cuda/targets/x86_64-linux/lib/libcudnn_ops_infer.so.8
sudo ln -sf /usr/local/cuda/targets/x86_64-linux/lib/libcudnn_cnn_train.so.8.4.1 /usr/local/cuda/targets/x86_64-linux/lib/libcudnn_cnn_train.so.8
sudo ln -sf /usr/local/cuda/targets/x86_64-linux/lib/libcudnn_adv_infer.so.8.4.1 /usr/local/cuda/targets/x86_64-linux/lib/libcudnn_adv_infer.so.8

# 安装TensorRT
echo -e "\033[32m[INFO]: TensorRT installing ...\033[0m"
sudo dpkg -i $tensorrt_fn
sudo apt-key add /var/nv-tensorrt-repo-ubuntu2004-cuda11.6-trt8.4.0.6-ea-20220212/7fa2af80.pub
sudo apt-get update
sudo apt-get install tensorrt -y
sudo apt-get install python3-libnvinfer-dev -y

sudo rm $cuda_fn
sudo rm $cudnn_fn
sudo rm $tensorrt_fn

