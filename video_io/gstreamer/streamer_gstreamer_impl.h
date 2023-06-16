#ifndef __SV_STREAM_GSTREAMER_IMPL__
#define __SV_STREAM_GSTREAMER_IMPL__

#include "sv_core.h"
#include <opencv2/opencv.hpp>
#include <opencv2/aruco.hpp>
#include <opencv2/tracking.hpp>
#include <string>
#include <chrono>

#ifdef WITH_GSTREAMER
#include <arpa/inet.h>
#include <gst/gst.h>
#include <gst/rtsp-server/rtsp-server.h>
#include <netinet/in.h>  // for sockaddr_in
#endif


namespace sv {


class VideoStreamerGstreamerImpl
{
public:
  VideoStreamerGstreamerImpl();
  ~VideoStreamerGstreamerImpl();

  bool gstreamerSetup(VideoStreamerBase* base_);
  bool gstreamerIsOpened();
  void gstreamerWrite(cv::Mat img_);
  void gstreamerRelease();

  int _rtsp_port;
  std::string _url;
  int _bitrate;
  cv::Size _stream_size;

#ifdef WITH_GSTREAMER
  cv::VideoWriter _stream_writer;
  GstRTSPServer *_server;
  GstRTSPMountPoints *_mounts;
  GstRTSPMediaFactory *_factory;
#endif
};


}
#endif
