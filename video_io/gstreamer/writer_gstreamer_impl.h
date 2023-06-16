#ifndef __SV_WRITER_GSTREAMER_IMPL__
#define __SV_WRITER_GSTREAMER_IMPL__

#include "sv_core.h"
#include <opencv2/opencv.hpp>
#include <opencv2/aruco.hpp>
#include <opencv2/tracking.hpp>
#include <string>
#include <chrono>


namespace sv {


class VideoWriterGstreamerImpl
{
public:
  VideoWriterGstreamerImpl();
  ~VideoWriterGstreamerImpl();

  bool gstreamerSetup(VideoWriterBase* base_, std::string file_name_);
  bool gstreamerIsOpened();
  void gstreamerWrite(cv::Mat img_);
  void gstreamerRelease();
  
  std::string _file_path;
  double _fps;
  cv::Size _image_size;

#ifdef WITH_GSTREAMER
  cv::VideoWriter _writer;
#endif
};


}
#endif
