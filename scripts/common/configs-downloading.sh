#!/bin/bash -e

root_dir=${HOME}"/SpireCV/models"
root_server="https://download.amovlab.com/model"

sv_params1=${HOME}"/SpireCV/sv_algorithm_params.json"
sv_params2=${HOME}"/SpireCV/sv_algorithm_params_coco_640.json"
sv_params3=${HOME}"/SpireCV/sv_algorithm_params_coco_1280.json"
camera_params1=${HOME}"/SpireCV/calib_webcam_640x480.yaml"
camera_params2=${HOME}"/SpireCV/calib_webcam_1280x720.yaml"


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

