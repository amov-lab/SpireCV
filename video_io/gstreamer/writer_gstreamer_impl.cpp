#include "writer_gstreamer_impl.h"
#include <cmath>
#include <fstream>



namespace sv {


VideoWriterGstreamerImpl::VideoWriterGstreamerImpl()
{
}
VideoWriterGstreamerImpl::~VideoWriterGstreamerImpl()
{
}

bool VideoWriterGstreamerImpl::gstreamerSetup(VideoWriterBase* base_, std::string file_name_)
{
  this->_file_path = base_->getFilePath();
  this->_fps = base_->getFps();
  this->_image_size = base_->getSize();

#ifdef WITH_GSTREAMER
  bool opend = false;
#ifdef PLATFORM_JETSON
    std::string pipeline = "appsrc ! videoconvert ! nvvidconv ! video/x-raw(memory:NVMM) ! nvv4l2h264enc ! h264parse ! matroskamux ! filesink location=" + this->_file_path + file_name_ + ".avi";
    opend = this->_writer.open(pipeline, cv::VideoWriter::fourcc('m','p','4','v'), this->_fps, this->_image_size);
#else
    opend = this->_writer.open(this->_file_path + file_name_ + ".avi", cv::VideoWriter::fourcc('x','v','i','d'), this->_fps, this->_image_size);
#endif
  return opend;
#endif
  return false;
}

bool VideoWriterGstreamerImpl::gstreamerIsOpened()
{
#ifdef WITH_GSTREAMER
  return this->_writer.isOpened();
#endif
  return false;
}

void VideoWriterGstreamerImpl::gstreamerWrite(cv::Mat img_)
{
#ifdef WITH_GSTREAMER
  this->_writer << img_;
#endif
}

void VideoWriterGstreamerImpl::gstreamerRelease()
{
#ifdef WITH_GSTREAMER
  if (this->_writer.isOpened())
    this->_writer.release();
#endif
}


}

