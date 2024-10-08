#include "sv_video_input.h"
#include <cmath>
#include <fstream>

namespace sv {

Camera::Camera()
{
}

Camera::~Camera()
{
}

void Camera::openImpl()
{
  if (this->_type == CameraType::WEBCAM || this->_type == CameraType::V4L2CAM)
  {
    if (this->_type == CameraType::V4L2CAM)
    {
      this->_cap.open(this->_camera_id, cv::CAP_V4L2);
    }
    if (this->_type == CameraType::WEBCAM)
    {
      this->_cap.open(this->_camera_id);
    }
    if (_fourcc.size() >= 4)
    {
      const char *fourcc_cstr = _fourcc.c_str();
      this->_cap.set(cv::CAP_PROP_FOURCC, cv::VideoWriter::fourcc(fourcc_cstr[0], fourcc_cstr[1], fourcc_cstr[2], fourcc_cstr[3]));
    }
    if (this->_width > 0 && this->_height > 0)
    {
      this->_cap.set(cv::CAP_PROP_FRAME_WIDTH, this->_width);
      this->_cap.set(cv::CAP_PROP_FRAME_HEIGHT, this->_height);
    }
    if (this->_fps > 0)
    {
      this->_cap.set(cv::CAP_PROP_FPS, this->_fps);
    }
    if (this->_brightness > 0)
    {
      this->_cap.set(cv::CAP_PROP_BRIGHTNESS, this->_brightness);
    }
    if (this->_contrast > 0)
    {
      this->_cap.set(cv::CAP_PROP_CONTRAST, this->_contrast);
    }
    if (this->_saturation > 0)
    {
      this->_cap.set(cv::CAP_PROP_SATURATION, this->_saturation);
    }
    if (this->_hue > 0)
    {
      this->_cap.set(cv::CAP_PROP_HUE, this->_hue);
    }
    if (this->_exposure > 0)
    {
      this->_cap.set(cv::CAP_PROP_EXPOSURE, this->_exposure);
    }
  }
  else if (this->_type == CameraType::G1)
  {
    char pipe[512];
    if (this->_width <= 0 || this->_height <= 0)
    {
      this->_width = 1280;
      this->_height = 720;
    }
    if (this->_port <= 0)
    {
      this->_port = 554;
    }
    if (this->_fps <= 0)
    {
      this->_fps = 30;
    }

#if defined(PLATFORM_X86_CUDA) || defined(PLATFORM_X86_INTEL)
    sprintf(pipe, "rtspsrc location=rtsp://%s:%d/H264?W=%d&H=%d&FPS=%d&BR=4000000 latency=100 ! \
                   application/x-rtp,media=video ! rtph264depay ! parsebin ! avdec_h264 ! \
                   videoconvert ! appsink sync=false", 
                   this->_ip.c_str(), this->_port, this->_width, this->_height, this->_fps);
#endif
#ifdef PLATFORM_JETSON
    sprintf(pipe, "rtspsrc location=rtsp://%s:%d/H264?W=%d&H=%d&FPS=%d&BR=4000000 latency=100 ! \
                   application/x-rtp,media=video ! rtph264depay ! parsebin ! nvv4l2decoder enable-max-performancegst=1 ! \
                   nvvidconv ! video/x-raw,format=(string)BGRx ! videoconvert ! appsink sync=false", 
                   this->_ip.c_str(), this->_port, this->_width, this->_height, this->_fps);
#endif

    // printf("%s\r\n",pipe);
    this->_cap.open(pipe, cv::CAP_GSTREAMER);
  }
  else if (this->_type == CameraType::GX40)
  {
    std::ostringstream camera_url;
    if (this->_width <= 0 || this->_height <= 0)
    {
      this->_width = 1280;
      this->_height = 720;
    }
    if (this->_port <= 0)
    {
      this->_port = 554;
    }
#if defined(PLATFORM_X86_CUDA) || defined(PLATFORM_X86_INTEL)
    camera_url << "rtspsrc location = rtsp://user:0000@" << this->_ip << ":" << this->_port
               << "/cam/realmonitor?channel=1&subtype=0 latency=100 ! application/x-rtp,media=video ! \
               rtph265depay ! parsebin ! avdec_h265 ! videoscale ! video/x-raw,width=(int)" << this->_width
               << ",height=(int)" << this->_height << " ! videoflip video-direction=4 ! videoconvert ! video/x-raw,format=(string)BGR ! \
               appsink sync=false";
      this->_cap.open(camera_url.str(), cv::CAP_GSTREAMER);
#endif
#ifdef PLATFORM_JETSON
    camera_url << "rtspsrc location = rtsp://user:0000@" << this->_ip << ":" << this->_port
               << "/cam/realmonitor?channel=1&subtype=0 latency=100 ! \
               application/x-rtp,media=video ! rtph265depay ! parsebin ! \
               nvv4l2decoder ! nvvidconv ! \
               video/x-raw,format=(string)BGRx,width=(int)"
               << this->_width << ",height=(int)" << this->_height << " ! videoconvert ! video/x-raw,format=(string)BGR ! \
               appsink sync=false";
    this->_cap.open(camera_url.str(), cv::CAP_GSTREAMER);
#endif
  }
  else if (this->_type == CameraType::MIPI)
  {
    char pipe[512];
    if (this->_width <= 0 || this->_height <= 0)
    {
      this->_width = 1280;
      this->_height = 720;
    }
    if (this->_fps <= 0)
    {
      this->_fps = 30;
    }

    sprintf(pipe, "nvarguscamerasrc sensor-id=%d ee-mode=0 tnr-mode=0 aeantibanding=0 wbmode=0 ! \
                   video/x-raw(memory:NVMM), width=(int)%d, height=(int)%d, format=(string)NV12, framerate=(fraction)%d/1 ! \
                   nvvidconv ! video/x-raw, format=(string)BGRx ! videoconvert ! video/x-raw, format=(string)BGR ! appsink",
                   this->_camera_id, this->_width, this->_height, this->_fps);
    this->_cap.open(pipe, cv::CAP_GSTREAMER);
  }
  else if (this->_type == CameraType::RTSP)
  {
    char pipe[512];
    if (this->_width <= 0 || this->_height <= 0)
    {
      this->_width = 1280;
      this->_height = 720;
    }
    if (this->_port <= 0)
    {
      this->_port = 554;
    }
    if (this->_fps <= 0)
    {
      this->_fps = 30;
    }

#if defined(PLATFORM_X86_CUDA)
    sprintf(pipe, "rtspsrc location=%s?W=%d&H=%d&FPS=%d latency=100 ! \
                   application/x-rtp,media=video ! rtph264depay ! parsebin ! avdec_h264 ! \
                   videoconvert ! appsink sync=false", 
                   this->_rtsp_url.c_str(), this->_width, this->_height, this->_fps);
    this->_cap.open(pipe, cv::CAP_GSTREAMER);
#endif
#if defined(PLATFORM_X86_INTEL)
    sprintf(pipe, "rtspsrc location=%s?W=%d&H=%d&FPS=%d latency=100 ! \
                   application/x-rtp,media=video ! rtph264depay ! parsebin ! vaapih264dec ! \
                   vaapipostproc ! video/x-raw,format=YV12,framerate=25/1 ! videoconvert ! videorate ! video/x-raw,framerate=25/1 ! \
                   appsink sync=false",
                   this->_rtsp_url.c_str(), this->_width, this->_height, this->_fps);
    this->_cap.open(pipe, cv::CAP_GSTREAMER);
#endif


#ifdef PLATFORM_JETSON
    sprintf(pipe, "rtspsrc location=%s?W=%d&H=%d&FPS=%d latency=100 ! application/x-rtp,media=video ! rtph264depay ! parsebin ! nvv4l2decoder enable-max-performancegst=1 ! nvvidconv ! video/x-raw,format=(string)BGRx ! videoconvert ! appsink sync=false", this->_rtsp_url.c_str(), this->_width, this->_height, this->_fps);
    this->_cap.open(pipe, cv::CAP_GSTREAMER);
#endif
  }
  else if (this->_type == CameraType::VIDEO)
  {
    this->_cap.open(this->_video_path);
  }
}

}
