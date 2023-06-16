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
  if (this->_type == CameraType::WEBCAM)
  {
    this->_cap.open(this->_camera_id);
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

    sprintf(pipe, "rtspsrc location=rtsp://%s:%d/H264?W=%d&H=%d&FPS=%d&BR=4000000 latency=100 ! application/x-rtp,media=video ! rtph264depay ! parsebin ! nvv4l2decoder enable-max-performancegst=1 ! nvvidconv ! video/x-raw,format=(string)BGRx ! videoconvert ! appsink sync=false", this->_ip.c_str(), this->_port, this->_width, this->_height, this->_fps);
    this->_cap.open(pipe, cv::CAP_GSTREAMER);
  }
}


}
