cmake_minimum_required(VERSION 3.0 FATAL_ERROR)
cmake_policy(SET CMP0054 NEW)

set(PROJECT_VERSION 0.2.0)
project(SpireCV VERSION ${PROJECT_VERSION} LANGUAGES CXX)

add_definitions(-DAPI_EXPORTS)
set(CMAKE_BUILD_TYPE "Release")


## JETSON, X86_CUDA, X86_INTEL
message(STATUS "System:${CMAKE_HOST_SYSTEM_PROCESSOR}")
if(NOT DEFINED PLATFORM)
  message(FATAL_ERROR "PLATFORM NOT SPECIFIED!")
else()
  message(STATUS "PLATFORM: ${PLATFORM}")
  if(PLATFORM STREQUAL "JETSON")
    add_definitions(-DPLATFORM_JETSON)
    option(USE_CUDA "BUILD WITH CUDA." ON)
    option(USE_GSTREAMER "BUILD WITH GSTREAMER." ON)
  elseif(PLATFORM STREQUAL "X86_CUDA")
    add_definitions(-DPLATFORM_X86_CUDA)
    option(USE_CUDA "BUILD WITH CUDA." ON)
    option(USE_FFMPEG "BUILD WITH FFMPEG." ON)
  elseif(PLATFORM STREQUAL "X86_INTEL")
    add_definitions(-DPLATFORM_X86_INTEL)
    option(USE_FFMPEG "BUILD WITH FFMPEG." ON)
  else()
    message(FATAL_ERROR "UNSUPPORTED PLATFORM!")
  endif()
endif()


if(USE_CUDA)
  add_definitions(-DWITH_CUDA)
  option(CUDA_USE_STATIC_CUDA_RUNTIME OFF)
  find_package(CUDA REQUIRED)
  message(STATUS "CUDA: ON")
endif()


if(USE_GSTREAMER)
  add_definitions(-DWITH_GSTREAMER)
  message(STATUS "GSTREAMER: ON")
endif()

if(USE_FFMPEG)
  add_definitions(-DWITH_FFMPEG)
  find_package(fmt REQUIRED)
  set(FFMPEG_LIBS libavutil.so libavcodec.so libavformat.so libavdevice.so libavfilter.so libswscale.so)
  message(STATUS "WITH_FFMPEG: ON")
endif()


find_package(Eigen3 REQUIRED)
add_definitions(-DWITH_OCV470)
find_package(OpenCV 4.7 REQUIRED)
message(STATUS "OpenCV library status:")
message(STATUS "    version: ${OpenCV_VERSION}")
message(STATUS "    libraries: ${OpenCV_LIBS}")
message(STATUS "    include path: ${OpenCV_INCLUDE_DIRS}")


include_directories(${EIGEN3_INCLUDE_DIRS})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/gimbal_ctrl/IOs/serial/include
  ${CMAKE_CURRENT_SOURCE_DIR}/gimbal_ctrl/driver/src/FIFO
  ${CMAKE_CURRENT_SOURCE_DIR}/gimbal_ctrl/driver/src/G1
  ${CMAKE_CURRENT_SOURCE_DIR}/gimbal_ctrl/driver/src/G2
  ${CMAKE_CURRENT_SOURCE_DIR}/gimbal_ctrl/driver/src/Q10f
  ${CMAKE_CURRENT_SOURCE_DIR}/gimbal_ctrl/driver/src
  ${CMAKE_CURRENT_SOURCE_DIR}/gimbal_ctrl
  ${CMAKE_CURRENT_SOURCE_DIR}/algorithm/common_det/cuda
  ${CMAKE_CURRENT_SOURCE_DIR}/algorithm/landing_det/cuda
  ${CMAKE_CURRENT_SOURCE_DIR}/algorithm/sot/ocv470
  ${CMAKE_CURRENT_SOURCE_DIR}/algorithm/color_line
  ${CMAKE_CURRENT_SOURCE_DIR}/video_io
    ${CMAKE_CURRENT_SOURCE_DIR}/algorithm/veri/cuda
  ${CMAKE_CURRENT_SOURCE_DIR}/algorithm/ellipse_det
  ${CMAKE_CURRENT_SOURCE_DIR}/utils
)

if(USE_GSTREAMER)
  include_directories(${CMAKE_CURRENT_SOURCE_DIR}/video_io/gstreamer)
  if(CMAKE_HOST_SYSTEM_PROCESSOR MATCHES "aarch64")
    include_directories(
      "/usr/include/gstreamer-1.0"
      "/usr/local/include/gstreamer-1.0"
      "/usr/include/glib-2.0"
      "/usr/lib/aarch64-linux-gnu/glib-2.0/include"
    )
  elseif(CMAKE_HOST_SYSTEM_PROCESSOR MATCHES "x86_64")
    include_directories(
      "/usr/include/gstreamer-1.0"
      "/usr/local/include/gstreamer-1.0"
      "/usr/include/glib-2.0"
      "/usr/lib/x86_64-linux-gnu/glib-2.0/include"
    )
  endif()
endif()

if(USE_FFMPEG)
  include_directories(${CMAKE_CURRENT_SOURCE_DIR}/video_io/ffmpeg)
endif()


# Public header
set(
  public_HEADS
  include/sv_core.h
  include/sv_video_base.h
  include/sv_gimbal.h
  include/sv_algorithm_base.h
  include/sv_common_det.h
  include/sv_landing_det.h
  include/sv_sot.h
  include/sv_mot.h
  include/sv_color_line.h
  include/sv_veri_det.h
  include/sv_video_input.h
  include/sv_video_output.h
  include/sv_world.h
)

# Gimbal Sources
set(serial_SRCS
  gimbal_ctrl/IOs/serial/src/serial.cc
)
list(APPEND serial_SRCS gimbal_ctrl/IOs/serial/src/impl/unix.cc)
list(APPEND serial_SRCS gimbal_ctrl/IOs/serial/src/impl/list_ports/list_ports_linux.cc)

set(driver_SRCS
  gimbal_ctrl/driver/src/FIFO/Ring_Fifo.cc
)
file(GLOB DRV_LIB_FILES ${CMAKE_CURRENT_SOURCE_DIR}/gimbal_ctrl/driver/src/G1/*.cpp)
list(APPEND driver_SRCS ${DRV_LIB_FILES})
file(GLOB DRV_LIB_FILES ${CMAKE_CURRENT_SOURCE_DIR}/gimbal_ctrl/driver/src/G2/*.cpp)
list(APPEND driver_SRCS ${DRV_LIB_FILES})
file(GLOB DRV_LIB_FILES ${CMAKE_CURRENT_SOURCE_DIR}/gimbal_ctrl/driver/src/Q10f/*.cpp)
list(APPEND driver_SRCS ${DRV_LIB_FILES})
file(GLOB DRV_LIB_FILES ${CMAKE_CURRENT_SOURCE_DIR}/gimbal_ctrl/driver/src/*.cpp)
list(APPEND driver_SRCS ${DRV_LIB_FILES})

set(gimbal_SRCS
  gimbal_ctrl/sv_gimbal.cpp
  gimbal_ctrl/sv_gimbal_io.hpp
)

# Gimbal Lib
add_library(sv_gimbal SHARED ${serial_SRCS} ${driver_SRCS} ${gimbal_SRCS})
target_link_libraries(sv_gimbal rt pthread)


set(spirecv_SRCS
  algorithm/sv_algorithm_base.cpp
  algorithm/ellipse_det/ellipse_detector.cpp
  algorithm/common_det/sv_common_det.cpp
  algorithm/landing_det/sv_landing_det.cpp
  algorithm/veri/sv_veri_det.cpp
  algorithm/sot/sv_sot.cpp
  algorithm/mot/sv_mot.cpp
  algorithm/color_line/sv_color_line.cpp
)

file(GLOB ALG_SRC_FILES ${CMAKE_CURRENT_SOURCE_DIR}/algorithm/sot/ocv470/*.cpp)
list(APPEND spirecv_SRCS ${ALG_SRC_FILES})
file(GLOB ALG_SRC_FILES ${CMAKE_CURRENT_SOURCE_DIR}/algorithm/color_line/*.cpp)
list(APPEND spirecv_SRCS ${ALG_SRC_FILES})
file(GLOB ALG_SRC_FILES ${CMAKE_CURRENT_SOURCE_DIR}/video_io/*.cpp)
list(APPEND spirecv_SRCS ${ALG_SRC_FILES})
file(GLOB ALG_SRC_FILES ${CMAKE_CURRENT_SOURCE_DIR}/utils/*.cpp)
list(APPEND spirecv_SRCS ${ALG_SRC_FILES})

if(USE_CUDA)
  list(APPEND spirecv_SRCS algorithm/common_det/cuda/yolov7/preprocess.cu)
  file(GLOB ALG_SRC_FILES ${CMAKE_CURRENT_SOURCE_DIR}/algorithm/common_det/cuda/*.cpp)
  list(APPEND spirecv_SRCS ${ALG_SRC_FILES})
  file(GLOB ALG_SRC_FILES ${CMAKE_CURRENT_SOURCE_DIR}/algorithm/common_det/cuda/yolov7/*.cpp)
  list(APPEND spirecv_SRCS ${ALG_SRC_FILES})
  file(GLOB ALG_SRC_FILES ${CMAKE_CURRENT_SOURCE_DIR}/algorithm/landing_det/cuda/*.cpp)
  list(APPEND spirecv_SRCS ${ALG_SRC_FILES})
  file(GLOB ALG_SRC_FILES ${CMAKE_CURRENT_SOURCE_DIR}/algorithm/veri/cuda/*.cpp)
  list(APPEND spirecv_SRCS ${ALG_SRC_FILES})
endif()

if(USE_FFMPEG)
  file(GLOB ALG_SRC_FILES ${CMAKE_CURRENT_SOURCE_DIR}/video_io/ffmpeg/*.cpp)
  list(APPEND spirecv_SRCS ${ALG_SRC_FILES})
endif()

if(USE_GSTREAMER)
file(GLOB ALG_SRC_FILES ${CMAKE_CURRENT_SOURCE_DIR}/video_io/gstreamer/*.cpp)
list(APPEND spirecv_SRCS ${ALG_SRC_FILES})
endif()


if(USE_CUDA)  # PLATFORM_X86_CUDA & PLATFORM_JETSON
  # CUDA
  include_directories(/usr/local/cuda/include)
  link_directories(/usr/local/cuda/lib64)
  # TensorRT
  include_directories(/usr/include/x86_64-linux-gnu)
  link_directories(/usr/lib/x86_64-linux-gnu)
  # Add library
  cuda_add_library(sv_yoloplugins SHARED algorithm/common_det/cuda/yolov7/yololayer.cu)
  target_link_libraries(sv_yoloplugins nvinfer cudart)
  
  cuda_add_library(sv_world SHARED ${spirecv_SRCS})
  target_link_libraries(
    sv_world ${OpenCV_LIBS} 
    sv_yoloplugins sv_gimbal
    nvinfer cudart
  )
  
  set(
    YOLO_SRCS
    algorithm/common_det/cuda/yolov7/preprocess.cu
    algorithm/common_det/cuda/yolov7/postprocess.cpp
    algorithm/common_det/cuda/yolov7/model.cpp
    algorithm/common_det/cuda/yolov7/calibrator.cpp
  )

  cuda_add_executable(SpireCVDet samples/SpireCVDet.cpp ${YOLO_SRCS})
  target_link_libraries(SpireCVDet sv_world)

  cuda_add_executable(SpireCVSeg samples/SpireCVSeg.cpp ${YOLO_SRCS})
  target_link_libraries(SpireCVSeg sv_world)

elseif(PLATFORM STREQUAL "X86_INTEL")  # Links to Intel-OpenVINO libraries here
  add_library(sv_world SHARED ${spirecv_SRCS})
  target_link_libraries(
    sv_world ${OpenCV_LIBS} 
    sv_gimbal
  )
endif()

if(USE_GSTREAMER)
  target_link_libraries(sv_world gstrtspserver-1.0)
endif()

if(USE_FFMPEG)
  target_link_libraries(sv_world ${FFMPEG_LIBS} fmt)
endif()

#demo
add_executable(ArucoDetection samples/demo/aruco_detection.cpp)
target_link_libraries(ArucoDetection sv_world)
add_executable(CameraReading samples/demo/camera_reading.cpp)
target_link_libraries(CameraReading sv_world)
add_executable(CommonObjectDetection samples/demo/common_object_detection.cpp)
target_link_libraries(CommonObjectDetection sv_world)
add_executable(DetectionWithClickedTracking samples/demo/detection_with_clicked_tracking.cpp)
target_link_libraries(DetectionWithClickedTracking sv_world)
add_executable(EllipseDetection samples/demo/ellipse_detection.cpp)
target_link_libraries(EllipseDetection sv_world)
add_executable(LandingMarkerDetection samples/demo/landing_marker_detection.cpp)
target_link_libraries(LandingMarkerDetection sv_world)
add_executable(SingleObjectTracking samples/demo/single_object_tracking.cpp)
target_link_libraries(SingleObjectTracking sv_world)
add_executable(MultipleObjectTracking samples/demo/multiple_object_tracking.cpp)
target_link_libraries(MultipleObjectTracking sv_world)
add_executable(ColorLineDetection samples/demo/color_line_detect.cpp)
target_link_libraries(ColorLineDetection sv_world)
add_executable(UdpDetectionInfoReceiver samples/demo/udp_detection_info_receiver.cpp)
target_link_libraries(UdpDetectionInfoReceiver sv_world)
add_executable(UdpDetectionInfoSender samples/demo/udp_detection_info_sender.cpp)
target_link_libraries(UdpDetectionInfoSender sv_world)
add_executable(VideoSaving samples/demo/video_saving.cpp)
target_link_libraries(VideoSaving sv_world)
add_executable(VERI samples/demo/veri.cpp)
target_link_libraries(VERI sv_world)
add_executable(VideoStreaming samples/demo/video_streaming.cpp)
target_link_libraries(VideoStreaming sv_world)
add_executable(GimbalClickedTracking samples/demo/gimbal_detection_with_clicked_tracking.cpp)
target_link_libraries(GimbalClickedTracking sv_world)
add_executable(GimbalLandingMarkerDetection samples/demo/gimbal_landing_marker_detection.cpp)
target_link_libraries(GimbalLandingMarkerDetection sv_world)
add_executable(GimbalUdpDetectionInfoSender samples/demo/gimbal_udp_detection_info_sender.cpp)
target_link_libraries(GimbalUdpDetectionInfoSender sv_world)

add_executable(EvalFpsOnVideo samples/test/eval_fps_on_video.cpp)
target_link_libraries(EvalFpsOnVideo sv_world)

add_executable(GimbalTest samples/test/gimbal_test.cpp)
target_link_libraries(GimbalTest sv_world)

add_executable(EvalModelOnCocoVal samples/test/eval_mAP_on_coco_val/eval_mAP_on_coco_val.cpp)
target_link_libraries(EvalModelOnCocoVal sv_world)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/samples/calib)
add_executable(CameraCalibrarion samples/calib/calibrate_camera_charuco.cpp)
target_link_libraries(CameraCalibrarion ${OpenCV_LIBS})

message(STATUS "CMAKE_INSTALL_PREFIX: ${CMAKE_INSTALL_PREFIX}")
if (NOT DEFINED SV_INSTALL_PREFIX)
  set(SV_INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX})
  message(STATUS "SV_INSTALL_PREFIX: ${SV_INSTALL_PREFIX}")
else()
  message(STATUS "SV_INSTALL_PREFIX: ${SV_INSTALL_PREFIX}")
endif()


if(USE_CUDA)
  install(TARGETS sv_gimbal sv_yoloplugins sv_world
    LIBRARY DESTINATION lib
  )
  install(TARGETS SpireCVDet SpireCVSeg
    RUNTIME DESTINATION bin
  )
elseif(PLATFORM STREQUAL "X86_INTEL")
  install(TARGETS sv_world
    LIBRARY DESTINATION lib
  )
endif()

install(FILES ${public_HEADS} 
  DESTINATION include
)


if(PLATFORM STREQUAL "JETSON")
file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/build/${PROJECT_NAME}Config.cmake.in [[
@PACKAGE_INIT@
find_package(OpenCV 4 REQUIRED)
link_directories(/usr/local/cuda/lib64)
set(SV_INCLUDE_DIRS
  @SV_INSTALL_PREFIX@/include
  /usr/include/x86_64-linux-gnu
  /usr/local/cuda/include
  ${OpenCV_INCLUDE_DIRS}
  /usr/include/gstreamer-1.0
  /usr/local/include/gstreamer-1.0
  /usr/include/glib-2.0
  /usr/lib/aarch64-linux-gnu/glib-2.0/include
)
set(SV_LIBRARIES
  @SV_INSTALL_PREFIX@/lib/libsv_yoloplugins.so
  @SV_INSTALL_PREFIX@/lib/libsv_world.so
  @SV_INSTALL_PREFIX@/lib/libsv_gimbal.so
  ${OpenCV_LIBS}
  nvinfer cudart rt pthread
  gstrtspserver-1.0
)
]])
elseif(PLATFORM STREQUAL "X86_CUDA")
file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/build/${PROJECT_NAME}Config.cmake.in [[
@PACKAGE_INIT@
find_package(OpenCV 4 REQUIRED)
find_package(fmt REQUIRED)
link_directories(/usr/local/cuda/lib64)
set(SV_INCLUDE_DIRS
  @SV_INSTALL_PREFIX@/include
  /usr/include/x86_64-linux-gnu
  /usr/local/cuda/include
  ${OpenCV_INCLUDE_DIRS}
)
set(SV_LIBRARIES
  @SV_INSTALL_PREFIX@/lib/libsv_yoloplugins.so
  @SV_INSTALL_PREFIX@/lib/libsv_world.so
  @SV_INSTALL_PREFIX@/lib/libsv_gimbal.so
  ${OpenCV_LIBS}
  nvinfer cudart rt pthread
  @FFMPEG_LIBS@ fmt
)
]])
elseif(PLATFORM STREQUAL "X86_INTEL")
file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/build/${PROJECT_NAME}Config.cmake.in [[
@PACKAGE_INIT@
find_package(OpenCV 4 REQUIRED)
find_package(fmt REQUIRED)
set(SV_INCLUDE_DIRS
  @SV_INSTALL_PREFIX@/include
  /usr/include/x86_64-linux-gnu
  ${OpenCV_INCLUDE_DIRS}
)
set(SV_LIBRARIES
  @SV_INSTALL_PREFIX@/lib/libsv_world.so
  @SV_INSTALL_PREFIX@/lib/libsv_gimbal.so
  ${OpenCV_LIBS}
  rt pthread
  @FFMPEG_LIBS@ fmt
)
]])
endif()


include(CMakePackageConfigHelpers)
write_basic_package_version_file(
  ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config-version.cmake
  VERSION ${PROJECT_VERSION}
  COMPATIBILITY AnyNewerVersion
)
configure_package_config_file(${CMAKE_CURRENT_BINARY_DIR}/build/${PROJECT_NAME}Config.cmake.in
  ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake
  INSTALL_DESTINATION lib/cmake/${PROJECT_NAME}
)
install(FILES 
  ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake
  ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config-version.cmake
  DESTINATION lib/cmake/${PROJECT_NAME}
)


