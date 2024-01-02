#!/bin/sh

echo "\033[32m[INFO]:\033[0m Please enter the folder path of the installation package: "
# package_dir="/home/jario/Downloads/nv"

wget https://download.amovlab.com/model/install/x86-nvidia/cuda-repo-ubuntu1804-11-1-local_11.1.1-455.32.00-1_amd64.deb
wget https://download.amovlab.com/model/install/x86-nvidia/cudnn-11.3-linux-x64-v8.2.1.32.tgz
wget https://download.amovlab.com/model/install/x86-nvidia/nv-tensorrt-repo-ubuntu1804-cuda11.3-trt8.0.1.6-ga-20210626_1-1_amd64.deb
wget https://download.amovlab.com/model/install/x86-nvidia/cuda-ubuntu1804.pin

package_dir="."

cuda_fn=$package_dir"/cuda-repo-ubuntu1804-11-1-local_11.1.1-455.32.00-1_amd64.deb"
cudnn_fn=$package_dir"/cudnn-11.3-linux-x64-v8.2.1.32.tgz"
tensorrt_fn=$package_dir"/nv-tensorrt-repo-ubuntu1804-cuda11.3-trt8.0.1.6-ga-20210626_1-1_amd64.deb"
tmp_dir="/tmp"

echo "\033[32m[INFO]: CUDA_PKG: \033[0m"$cuda_fn
echo "\033[32m[INFO]: CUDNN_PKG: \033[0m"$cudnn_fn
echo "\033[32m[INFO]: TENSORRT_PKG: \033[0m"$tensorrt_fn

# 所有文件都存在时，才会继续执行脚本
if [ ! -f "$cuda_fn" ]; then
  echo "\033[31m[ERROR]: CUDA_PKG not exist!: \033[0m"
  exit 1
fi

if [ ! -f "$cudnn_fn" ]; then
  echo "\033[31m[ERROR]: CUDNN_PKG not exist!: \033[0m"
  exit 1
fi

if [ ! -f "$tensorrt_fn" ]; then
  echo "\033[31m[ERROR]: TENSORRT_PKG not exist!: \033[0m"
  exit 1
fi

# 删除显卡驱动
# sudo apt-get remove nvidia-*

# 安装显卡驱动
# echo "\033[32m[INFO]: Nvidia Driver installing ...\033[0m"
# ubuntu-drivers devices
# sudo ubuntu-drivers autoinstall

# 删除已安装CUDA
# --purge选项会将配置文件、数据库等删除
# sudo apt-get autoremove --purge cuda
# 查看安装了哪些cuda相关的库，可以用以下指令
# sudo dpkg -l |grep cuda
# sudo dpkg -P cuda-repo-ubuntu1804-10-2-local-10.2.89-440.33.01
# sudo dpkg -P cuda-repo-ubuntu1804-11-1-local
# sudo dpkg -P nv-tensorrt-repo-ubuntu1804-cuda10.2-trt8.0.1.6-ga-20210626
# 这个key值是官网文档查到的，当然通过apt-key list也能查看
# sudo apt-key list
# sudo apt-key del 7fa2af80

# 安装CUDA
echo "\033[32m[INFO]: CUDA installing ...\033[0m"
# wget https://developer.download.nvidia.com/compute/cuda/repos/ubuntu1804/x86_64/cuda-ubuntu1804.pin
sudo cp $package_dir/cuda-ubuntu1804.pin /etc/apt/preferences.d/cuda-repository-pin-600
sudo dpkg -i $cuda_fn
sudo apt-key add /var/cuda-repo-ubuntu1804-11-1-local/7fa2af80.pub
sudo apt-get update
sudo apt-get -y install cuda

# 安装CUDNN
echo "\033[32m[INFO]: CUDNN installing ...\033[0m"
tar zxvf $cudnn_fn -C $tmp_dir
sudo cp $tmp_dir/cuda/include/cudnn* /usr/local/cuda/include/
sudo cp $tmp_dir/cuda/lib64/libcudnn* /usr/local/cuda/lib64/
sudo chmod a+r /usr/local/cuda/include/cudnn* /usr/local/cuda/lib64/libcudnn*

sudo ln -sf /usr/local/cuda/targets/x86_64-linux/lib/libcudnn_cnn_infer.so.8.2.1 /usr/local/cuda/targets/x86_64-linux/lib/libcudnn_cnn_infer.so.8
sudo ln -sf /usr/local/cuda/targets/x86_64-linux/lib/libcudnn.so.8.2.1 /usr/local/cuda/targets/x86_64-linux/lib/libcudnn.so.8
sudo ln -sf /usr/local/cuda/targets/x86_64-linux/lib/libcudnn_ops_train.so.8.2.1 /usr/local/cuda/targets/x86_64-linux/lib/libcudnn_ops_train.so.8
sudo ln -sf /usr/local/cuda/targets/x86_64-linux/lib/libcudnn_adv_train.so.8.2.1 /usr/local/cuda/targets/x86_64-linux/lib/libcudnn_adv_train.so.8
sudo ln -sf /usr/local/cuda/targets/x86_64-linux/lib/libcudnn_ops_infer.so.8.2.1 /usr/local/cuda/targets/x86_64-linux/lib/libcudnn_ops_infer.so.8
sudo ln -sf /usr/local/cuda/targets/x86_64-linux/lib/libcudnn_cnn_train.so.8.2.1 /usr/local/cuda/targets/x86_64-linux/lib/libcudnn_cnn_train.so.8
sudo ln -sf /usr/local/cuda/targets/x86_64-linux/lib/libcudnn_adv_infer.so.8.2.1 /usr/local/cuda/targets/x86_64-linux/lib/libcudnn_adv_infer.so.8

# 安装TensorRT
sudo dpkg -i $tensorrt_fn
sudo apt-key add /var/nv-tensorrt-repo-cuda11.3-trt8.0.1.6-ga-20210626/7fa2af80.pub
sudo apt-get update
sudo apt-get install tensorrt -y
sudo apt-get install python3-libnvinfer-dev -y

sudo rm $cuda_fn
sudo rm $cudnn_fn
sudo rm $tensorrt_fn
sudo rm cuda-ubuntu1804.pin



