#!/bin/bash -e

if [ ! -d build ]; then
  mkdir build
fi

cd build 

cmake ..
make -j

./EvalFpsOnVideo ${HOME}/SpireCV/test/tracking_640x480.mp4 ${HOME}/SpireCV/calib_webcam_640x480.yaml 51
./EvalFpsOnVideo ${HOME}/SpireCV/test/tracking_1280x720.mp4 ${HOME}/SpireCV/calib_webcam_1280x720.yaml 51
./EvalFpsOnVideo ${HOME}/SpireCV/test/tracking_1280x720.mp4 ${HOME}/SpireCV/calib_webcam_1920x1080.yaml 51

./EvalFpsOnVideo ${HOME}/SpireCV/test/tracking_640x480.mp4 ${HOME}/SpireCV/calib_webcam_640x480.yaml 52
./EvalFpsOnVideo ${HOME}/SpireCV/test/tracking_1280x720.mp4 ${HOME}/SpireCV/calib_webcam_1280x720.yaml 52
./EvalFpsOnVideo ${HOME}/SpireCV/test/tracking_1280x720.mp4 ${HOME}/SpireCV/calib_webcam_1920x1080.yaml 52

./EvalFpsOnVideo ${HOME}/SpireCV/test/tracking_640x480.mp4 ${HOME}/SpireCV/calib_webcam_640x480.yaml 54
./EvalFpsOnVideo ${HOME}/SpireCV/test/tracking_1280x720.mp4 ${HOME}/SpireCV/calib_webcam_1280x720.yaml 54
./EvalFpsOnVideo ${HOME}/SpireCV/test/tracking_1280x720.mp4 ${HOME}/SpireCV/calib_webcam_1920x1080.yaml 54

./EvalFpsOnVideo ${HOME}/SpireCV/test/drone_640x480.mp4 ${HOME}/SpireCV/calib_webcam_640x480.yaml 41
./EvalFpsOnVideo ${HOME}/SpireCV/test/drone_1280x720.mp4 ${HOME}/SpireCV/calib_webcam_1280x720.yaml 41
./EvalFpsOnVideo ${HOME}/SpireCV/test/drone_1280x720.mp4 ${HOME}/SpireCV/calib_webcam_1920x1080.yaml 41

./EvalFpsOnVideo ${HOME}/SpireCV/test/drone_640x480.mp4 ${HOME}/SpireCV/calib_webcam_640x480.yaml 42
./EvalFpsOnVideo ${HOME}/SpireCV/test/drone_1280x720.mp4 ${HOME}/SpireCV/calib_webcam_1280x720.yaml 42
./EvalFpsOnVideo ${HOME}/SpireCV/test/drone_1280x720.mp4 ${HOME}/SpireCV/calib_webcam_1920x1080.yaml 42

./EvalFpsOnVideo ${HOME}/SpireCV/test/drone_640x480.mp4 ${HOME}/SpireCV/calib_webcam_640x480.yaml 43
./EvalFpsOnVideo ${HOME}/SpireCV/test/drone_1280x720.mp4 ${HOME}/SpireCV/calib_webcam_1280x720.yaml 43
./EvalFpsOnVideo ${HOME}/SpireCV/test/drone_1280x720.mp4 ${HOME}/SpireCV/calib_webcam_1920x1080.yaml 43

./EvalFpsOnVideo ${HOME}/SpireCV/test/aruco_640x480.mp4 ${HOME}/SpireCV/calib_webcam_640x480.yaml 1
./EvalFpsOnVideo ${HOME}/SpireCV/test/aruco_1280x720.mp4 ${HOME}/SpireCV/calib_webcam_1280x720.yaml 1
./EvalFpsOnVideo ${HOME}/SpireCV/test/aruco_1280x720.mp4 ${HOME}/SpireCV/calib_webcam_1920x1080.yaml 1

./EvalFpsOnVideo ${HOME}/SpireCV/test/landing_640x480.mp4 ${HOME}/SpireCV/calib_webcam_640x480.yaml 2
./EvalFpsOnVideo ${HOME}/SpireCV/test/landing_1280x720.mp4 ${HOME}/SpireCV/calib_webcam_1280x720.yaml 2

./EvalFpsOnVideo ${HOME}/SpireCV/test/ellipse_640x480.mp4 ${HOME}/SpireCV/calib_webcam_640x480.yaml 3
./EvalFpsOnVideo ${HOME}/SpireCV/test/ellipse_1280x720.mp4 ${HOME}/SpireCV/calib_webcam_1280x720.yaml 3

