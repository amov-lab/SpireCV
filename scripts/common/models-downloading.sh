#!/bin/bash -e

root_dir=${HOME}"/SpireCV/models"
root_server="https://download.amovlab.com/model"

coco_model1="COCO-yolov5s.wts"
coco_model2="COCO-yolov5s6.wts"
coco_model3="COCO-yolov5s-seg.wts"
coco_model1_fn=${root_dir}/${coco_model1}
coco_model2_fn=${root_dir}/${coco_model2}
coco_model3_fn=${root_dir}/${coco_model3}

drone_model1="Drone-yolov5s-ap935-v230302.wts"
drone_model2="Drone-yolov5s6-ap949-v230302.wts"
drone_model1_fn=${root_dir}/${drone_model1}
drone_model2_fn=${root_dir}/${drone_model2}

personcar_model1="PersonCar-yolov5s-ap606-v230306.wts"
personcar_model2="PersonCar-yolov5s6-ap702-v230306.wts"
personcar_model1_fn=${root_dir}/${personcar_model1}
personcar_model2_fn=${root_dir}/${personcar_model2}

track_model1="dasiamrpn_model.onnx"
track_model2="dasiamrpn_kernel_cls1.onnx"
track_model3="dasiamrpn_kernel_r1.onnx"
track_model4="nanotrack_backbone_sim.onnx"
track_model5="nanotrack_head_sim.onnx"
track_model1_fn=${root_dir}/${track_model1}
track_model2_fn=${root_dir}/${track_model2}
track_model3_fn=${root_dir}/${track_model3}
track_model4_fn=${root_dir}/${track_model4}
track_model5_fn=${root_dir}/${track_model5}

landing_model1="LandingMarker-resnet34-v230228.onnx"
landing_model1_fn=${root_dir}/${landing_model1}

veri_model1="veri.onnx"
veri_model1_fn=${root_dir}/${veri_model1}

mde_model1="MonDepthEst_In.engine"
mde_model1_fn=${root_dir}/${mde_model1}

mde_model2="MonDepthEst_Out.engine"
mde_model2_fn=${root_dir}/${mde_model2}

if [ ! -d ${root_dir} ]; then
  echo -e "\033[32m[INFO]: ${root_dir} not exist, creating it ... \033[0m"
  mkdir -p ${root_dir}
fi

if [ ! -f ${coco_model1_fn} ]; then
  echo -e "\033[32m[INFO]: ${coco_model1_fn} not exist, downloading ... \033[0m"
  wget -O ${coco_model1_fn} ${root_server}/install/${coco_model1}
  wget -O ${coco_model2_fn} ${root_server}/install/${coco_model2}
  wget -O ${coco_model3_fn} ${root_server}/install/${coco_model3}
fi

if [ ! -f ${drone_model1_fn} ]; then
  echo -e "\033[32m[INFO]: ${drone_model1_fn} not exist, downloading ... \033[0m"
  wget -O ${drone_model1_fn} ${root_server}/install/${drone_model1}
  wget -O ${drone_model2_fn} ${root_server}/install/${drone_model2}
fi

if [ ! -f ${personcar_model1_fn} ]; then
  echo -e "\033[32m[INFO]: ${personcar_model1_fn} not exist, downloading ... \033[0m"
  wget -O ${personcar_model1_fn} ${root_server}/install/${personcar_model1}
  wget -O ${personcar_model2_fn} ${root_server}/install/${personcar_model2}
fi

if [ ! -f ${track_model1_fn} ]; then
  echo -e "\033[32m[INFO]: ${track_model1_fn} not exist, downloading ... \033[0m"
  wget -O ${track_model1_fn} ${root_server}/${track_model1}
  wget -O ${track_model2_fn} ${root_server}/${track_model2}
  wget -O ${track_model3_fn} ${root_server}/${track_model3}
fi

if [ ! -f ${track_model4_fn} ]; then
  echo -e "\033[32m[INFO]: ${track_model4_fn} not exist, downloading ... \033[0m"
  wget -O ${track_model4_fn} ${root_server}/${track_model4}
  wget -O ${track_model5_fn} ${root_server}/${track_model5}
fi

if [ ! -f ${landing_model1_fn} ]; then
  echo -e "\033[32m[INFO]: ${landing_model1_fn} not exist, downloading ... \033[0m"
  wget -O ${landing_model1_fn} ${root_server}/install/${landing_model1}
fi

if [ ! -f ${veri_model1_fn} ]; then
  echo -e "\033[32m[INFO]: ${veri_model1_fn} not exist, downloading ... \033[0m"
  wget -O ${veri_model1_fn} ${root_server}/install/${veri_model1}
fi

if [ ! -f ${mde_model1_fn} ]; then
  echo -e "\033[32m[INFO]: ${mde_model1_fn} not exist, downloading ... \033[0m"
  wget -O ${mde_model1_fn} ${root_server}/install/${mde_model1}
fi

if [ ! -f ${mde_model2_fn} ]; then
  echo -e "\033[32m[INFO]: ${mde_model2_fn} not exist, downloading ... \033[0m"
  wget -O ${mde_model2_fn} ${root_server}/install/${mde_model2}
fi
