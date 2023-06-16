#include "sv_video_output.h"
#include <cmath>
#include <fstream>
#ifdef WITH_GSTREAMER
#include "streamer_gstreamer_impl.h"
#include "writer_gstreamer_impl.h"
#endif
#ifdef WITH_FFMPEG
#include "bs_push_streamer.h"
#include "bs_video_saver.h"
#endif


namespace sv {


VideoWriter::VideoWriter()
{
#ifdef WITH_GSTREAMER
  this->_gstreamer_impl = new VideoWriterGstreamerImpl;
#endif
#ifdef WITH_FFMPEG
  this->_ffmpeg_impl = new BsVideoSaver;
#endif
}
VideoWriter::~VideoWriter()
{
}

bool VideoWriter::setupImpl(std::string file_name_)
{
  cv::Size img_sz = this->getSize();
  double fps = this->getFps();
  std::string file_path = this->getFilePath();

#ifdef WITH_GSTREAMER
  return this->_gstreamer_impl->gstreamerSetup(this, file_name_);
#endif
#ifdef WITH_FFMPEG
#ifdef PLATFORM_X86_CUDA
  std::string enc = "h264_nvenc";
#else
  std::string enc = "";
#endif
  return this->_ffmpeg_impl->setup(file_path + file_name_ + ".avi", img_sz.width, img_sz.height, (int)fps, enc, 4);
#endif
  return false;
}

bool VideoWriter::isOpenedImpl()
{
#ifdef WITH_GSTREAMER
  return this->_gstreamer_impl->gstreamerIsOpened();
#endif
#ifdef WITH_FFMPEG
  return this->isRunning();
#endif
  return false;
}

void VideoWriter::writeImpl(cv::Mat img_)
{
#ifdef WITH_GSTREAMER
  this->_gstreamer_impl->gstreamerWrite(img_);
#endif
#ifdef WITH_FFMPEG
  this->_ffmpeg_impl->write(img_);
#endif
}

void VideoWriter::releaseImpl()
{
#ifdef WITH_GSTREAMER
  this->_gstreamer_impl->gstreamerRelease();
#endif
#ifdef WITH_FFMPEG
  this->_ffmpeg_impl->stop();
#endif
}



VideoStreamer::VideoStreamer()
{
#ifdef WITH_GSTREAMER
  this->_gstreamer_impl = new VideoStreamerGstreamerImpl;
#endif
#ifdef WITH_FFMPEG
  this->_ffmpeg_impl = new BsPushStreamer;
#endif
}
VideoStreamer::~VideoStreamer()
{
}

bool VideoStreamer::setupImpl()
{
  cv::Size img_sz = this->getSize();
  int port = this->getPort();
  std::string url = this->getUrl();
  int bitrate = this->getBitrate();

#ifdef WITH_GSTREAMER
  return this->_gstreamer_impl->gstreamerSetup(this);
#endif
#ifdef WITH_FFMPEG
  std::string rtsp_url = "rtsp://127.0.0.1/live" + url;
#ifdef PLATFORM_X86_CUDA
  std::string enc = "h264_nvenc";
#else
  std::string enc = "";
#endif
  return this->_ffmpeg_impl->setup(rtsp_url, img_sz.width, img_sz.height, 24, enc, bitrate);
#endif
  return false;
}

bool VideoStreamer::isOpenedImpl()
{
#ifdef WITH_GSTREAMER
  return this->_gstreamer_impl->gstreamerIsOpened();
#endif
#ifdef WITH_FFMPEG
  return this->isRunning();
#endif
  return false;
}

void VideoStreamer::writeImpl(cv::Mat img_)
{
#ifdef WITH_GSTREAMER
  this->_gstreamer_impl->gstreamerWrite(img_);
#endif
#ifdef WITH_FFMPEG
  this->_ffmpeg_impl->stream(img_);
#endif
}

void VideoStreamer::releaseImpl()
{
#ifdef WITH_GSTREAMER
  this->_gstreamer_impl->gstreamerRelease();
#endif
#ifdef WITH_FFMPEG
  this->_ffmpeg_impl->stop();
#endif
}


}

