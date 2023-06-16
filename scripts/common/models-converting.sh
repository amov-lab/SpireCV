#!/bin/bash -e
export LD_LIBRARY_PATH=/usr/local/lib:$LD_LIBRARY_PATH

root_dir=${HOME}"/SpireCV/models"


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

landing_model1="LandingMarker-resnet34-v230228.onnx"
landing_model1_fn=${root_dir}/${landing_model1}

SpireCVDet -s ${coco_model1_fn} ${root_dir}/COCO.engine 80 s
SpireCVDet -s ${coco_model2_fn} ${root_dir}/COCO_HD.engine 80 s6
SpireCVSeg -s ${coco_model3_fn} ${root_dir}/COCO_SEG.engine 80 s

SpireCVDet -s ${drone_model1_fn} ${root_dir}/Drone.engine 1 s
SpireCVDet -s ${drone_model2_fn} ${root_dir}/Drone_HD.engine 1 s6

SpireCVDet -s ${personcar_model1_fn} ${root_dir}/PersonCar.engine 8 s
SpireCVDet -s ${personcar_model2_fn} ${root_dir}/PersonCar_HD.engine 8 s6

cd /usr/src/tensorrt/bin/
./trtexec --explicitBatch --onnx=${landing_model1_fn} --saveEngine=${root_dir}/LandingMarker.engine --fp16

echo "export LD_LIBRARY_PATH=/usr/local/lib:$LD_LIBRARY_PATH" >> ~/.bashrc

