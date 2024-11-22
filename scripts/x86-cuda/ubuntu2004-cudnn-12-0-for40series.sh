#!/bin/sh

echo -e "\033[32m[INFO]:\033[0m Please enter the folder path of the installation package: "
# package_dir="/home/jario/Downloads/nv"

wget https://download.amovlab.com/model/install/x86-nvidia/cuda_12.0.0_525.60.13_linux.run
wget https://download.amovlab.com/model/install/x86-nvidia/cudnn-linux-x86_64-8.9.7.29_cuda12-archive.tar.xz
wget https://download.amovlab.com/model/install/x86-nvidia/TensorRT-8.6.1.6.Linux.x86_64-gnu.cuda-12.0.tar.gz


#cudnn安装
package_dir="."
cudnn_fn= $package_dir"/cudnn-linux-x86_64-8.9.7.29_cuda12-archive"
tmp_dir= "/tmp"
echo -e "\033[32m[INFO]:CUDNN installing ...\033[0m"
tar -xvf $cudnn_fn -C $tmp_dir
sudo cp $tmp_dir/cudnn-linux-86_64-8.9.7.29_cuda12-archive/include/cudnn* /usr/local/cuda/include/
sudo cp $tmp_dir/cudnn-linux-86_64-8.9.7.29_cuda12-archive/lib/libcudnn* /usr/local/cuda/lib64/
sudo chmod a+r /usr/local/cuda/include/cudnn* /usr/local/cuda/lib64/libcudnn*

sudo ln -sf /usr/local/cuda/targets/x86_64-linux/lib/libcudnn_cnn_infer.so.8.9.7 /usr/local/cuda/targets/x86_64-linux/lib/libcudnn_cnn_infer.so.8
sudo ln -sf /usr/local/cuda/targets/x86_64-linux/lib/libcudnn.so.8.9.7 /usr/local/cuda/targets/x86_64-linux/lib/libcudnn.so.8
sudo ln -sf /usr/local/cuda/targets/x86_64-linux/lib/libcudnn_ops_train.so.8.9.7 /usr/local/cuda/targets/x86_64-linux/lib/libcudnn_ops_train.so.8
sudo ln -sf /usr/local/cuda/targets/x86_64-linux/lib/libcudnn_adv_train.so.8.9.7 /usr/local/cuda/targets/x86_64-linux/lib/libcudnn_adv_train.so.8
sudo ln -sf /usr/local/cuda/targets/x86_64-linux/lib/libcudnn_ops_infer.so.8.9.7 /usr/local/cuda/targets/x86_64-linux/lib/libcudnn_ops_infer.so.8
sudo ln -sf /usr/local/cuda/targets/x86_64-linux/lib/libcudnn_cnn_train.so.8.9.7 /usr/local/cuda/targets/x86_64-linux/lib/libcudnn_cnn_train.so.8
sudo ln -sf /usr/local/cuda/targets/x86_64-linux/lib/libcudnn_adv_infer.so.8.9.7 /usr/local/cuda/targets/x86_64-linux/lib/libcudnn_adv_infer.so.8

#tensorrt安装
tensorrt_fn="$package_dir/TensorRT-8.6.1.6.Linux.x86_64-gnu.cuda-12.0.tar.gz"
echo -e "\033[32m[INFO]:Tensorrt installing ...\033[0m"
# 获取 Python 版本
python_version=$(python3 --version | awk '{print $2}')
python_major=$(echo $python_version | cut -d '.' -f 2)

# 解压 TensorRT 文件
tar -xzvf $tensorrt_fn -C $HOME

# 设置环境变量
echo "export TENSORRT_DIR=\$HOME/TensorRT-8.6.1.6" >> ~/.bashrc
echo "export LD_LIBRARY_PATH=\$TENSORRT_DIR/lib:\$LD_LIBRARY_PATH" >> ~/.bashrc
echo "export LIBRARY_PATH=\$TENSORRT_DIR/lib:\$LIBRARY_PATH" >> ~/.bashrc

# 切换到 TensorRT Python 目录
cd $HOME/TensorRT-8.6.1.6/python

# 安装 TensorRT 相关包
python3 -m pip install tensorrt-8.6.1-cp3${python_major}-none-linux_x86_64.whl
python3 -m pip install tensorrt_lean-8.6.1-cp3${python_major}-none-linux_x86_64.whl
python3 -m pip install tensorrt_dispatch-8.6.1-cp3${python_major}-none-linux_x86_64.whl

# 设置其他 LD_LIBRARY_PATH 环境变量
echo "export PATH=/usr/local/cuda/bin\${PATH:+:\${PATH}}">> ~/.bashrc
echo "export LD_LIBRARY_PATH=/usr/local/cuda/lib64\${LD_LIBRARY_PATH:+:\${LD_LIBRARY_PATH}}">> ~/.bashrc
echo "export LD_LIBRARY_PATH=/usr/local/lib:/usr/local/lib:$HOME/TensorRT-8.6.1.6/lib:\$TENSORRT_DIR/lib:/usr/local/cuda/lib64" >> ~/.bashrc
echo "export LD_LIBRARY_PATH=$HOME/TensorRT-8.6.1.6/targets/x86_64-linux-gnu/lib:\$LD_LIBRARY_PATH">> ~/.bashrc
echo "export LD_LIBRARY_PATH=$HOME/.local/lib/python3.${python_major}/site-packages/nvidia/cusparse/lib:\$LD_LIBRARY_PATH" >> ~/.bashrc
echo "export LD_LIBRARY_PATH=$HOME/.local/lib/python3.${python_major}/site-packages/nvidia/cudnn/lib:\$LD_LIBRARY_PATH" >> ~/.bashrc

sudo rm $cudnn_fn
sudo rm $tensorrt_fn

# 结束
echo "Cudnn与Tensorrt安装完毕。"

