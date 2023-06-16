#!/bin/bash -e

root_dir=${HOME}"/SpireCV/models"
root_server="https://download.amovlab.com/model"

sv_params1=${HOME}"/SpireCV/sv_algorithm_params.json"
sv_params2=${HOME}"/SpireCV/sv_algorithm_params_coco_640.json"
sv_params3=${HOME}"/SpireCV/sv_algorithm_params_coco_1280.json"
camera_params1=${HOME}"/SpireCV/calib_webcam_640x480.yaml"
camera_params2=${HOME}"/SpireCV/calib_webcam_1280x720.yaml"

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


if [ ! -d ${root_dir} ]; then
  echo -e "\033[32m[INFO]: ${root_dir} not exist, creating it ... \033[0m"
  mkdir -p ${root_dir}
fi

if [ ! -f ${sv_params1} ]; then
  echo -e "\033[32m[INFO]: ${sv_params1} not exist, downloading ... \033[0m"
  wget -O ${sv_params1} ${root_server}/install/a-params/sv_algorithm_params.json
fi
if [ ! -f ${sv_params2} ]; then
  echo -e "\033[32m[INFO]: ${sv_params2} not exist, downloading ... \033[0m"
  wget -O ${sv_params2} ${root_server}/install/a-params/sv_algorithm_params_coco_640.json
fi
if [ ! -f ${sv_params3} ]; then
  echo -e "\033[32m[INFO]: ${sv_params3} not exist, downloading ... \033[0m"
  wget -O ${sv_params3} ${root_server}/install/a-params/sv_algorithm_params_coco_1280.json
fi

if [ ! -f ${camera_params1} ]; then
  echo -e "\033[32m[INFO]: ${camera_params1} not exist, downloading ... \033[0m"
  wget -O ${camera_params1} ${root_server}/install/c-params/calib_webcam_640x480.yaml
fi
if [ ! -f ${camera_params2} ]; then
  echo -e "\033[32m[INFO]: ${camera_params2} not exist, downloading ... \033[0m"
  wget -O ${camera_params2} ${root_server}/install/c-params/calib_webcam_1280x720.yaml
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

