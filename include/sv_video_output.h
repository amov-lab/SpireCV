#ifndef __SV_VIDEO_OUTPUT__
#define __SV_VIDEO_OUTPUT__

#include "sv_core.h"
#include <opencv2/opencv.hpp>
#include <opencv2/aruco.hpp>
#include <opencv2/tracking.hpp>
#include <string>
#include <chrono>


class BsVideoSaver;
class BsPushStreamer;

namespace sv {

class VideoWriterGstreamerImpl;
class VideoStreamerGstreamerImpl;

class VideoWriter : public VideoWriterBase
{
public:
  VideoWriter();
  ~VideoWriter();
protected:
  bool setupImpl(std::string file_name_);
  bool isOpenedImpl();
  void writeImpl(cv::Mat img_);
  void releaseImpl();

  VideoWriterGstreamerImpl* _gstreamer_impl;
  BsVideoSaver* _ffmpeg_impl;
};


class VideoStreamer : public VideoStreamerBase
{
public:
  VideoStreamer();
  ~VideoStreamer();
protected:
  bool setupImpl();
  bool isOpenedImpl();
  void writeImpl(cv::Mat img_);
  void releaseImpl();

  VideoStreamerGstreamerImpl* _gstreamer_impl;
  BsPushStreamer* _ffmpeg_impl;
};


}
#endif
