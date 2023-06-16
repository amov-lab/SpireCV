#include "streamer_gstreamer_impl.h"
#include <cmath>
#include <fstream>



namespace sv {


VideoStreamerGstreamerImpl::VideoStreamerGstreamerImpl()
{
}
VideoStreamerGstreamerImpl::~VideoStreamerGstreamerImpl()
{
}

bool VideoStreamerGstreamerImpl::gstreamerSetup(VideoStreamerBase* base_)
{
  this->_rtsp_port = base_->getPort();
  this->_url = base_->getUrl();
  this->_bitrate = base_->getBitrate();
  this->_stream_size = base_->getSize();

#ifdef WITH_GSTREAMER
  int media_port = 5400;
  char port_str[8];
  sprintf(port_str, "%d", this->_rtsp_port);

  /* create a server instance */
  this->_server = gst_rtsp_server_new();
  g_object_set(_server, "service", port_str, NULL);
  this->_mounts = gst_rtsp_server_get_mount_points(this->_server);
  this->_factory = gst_rtsp_media_factory_new();

  char media_str[512];

#ifdef PLATFORM_JETSON
  sprintf(media_str, "(udpsrc name=pay0 port=%d caps=\"application/x-rtp, media=(string)video, clock-rate=(int)90000, encoding-name=(string)H264, payload=96 \")", media_port);
  gst_rtsp_media_factory_set_launch(this->_factory, media_str);
  gst_rtsp_media_factory_set_shared(this->_factory, TRUE);
#else
  sprintf(media_str, "(udpsrc name=pay0 port=%d buffer-size=524288 caps=\"application/x-rtp, media=(string)video, clock-rate=(int)90000, encoding-name=(string)H264, payload=96 \")", media_port);
  gst_rtsp_media_factory_set_launch(this->_factory, media_str);
  gst_rtsp_media_factory_set_shared(this->_factory, TRUE);
#endif

  /* attach the test factory to the /test url */
  gst_rtsp_mount_points_add_factory(this->_mounts, this->_url.c_str(), this->_factory);
  /* don't need the ref to the mapper anymore */
  g_object_unref(this->_mounts);
  /* attach the server to the default maincontext */
  gst_rtsp_server_attach(this->_server, NULL);

  /* start serving */
  std::cout << "stream ready at rtsp://127.0.0.1:" << this->_rtsp_port << this->_url << std::endl;

  int bitrate = this->_bitrate;
  if (bitrate < 1)  bitrate = 1;
  if (bitrate > 20) bitrate = 20;

  char str_buf[512];

#ifdef PLATFORM_JETSON
  sprintf(str_buf, "appsrc is-live=true ! videoconvert ! nvvidconv ! video/x-raw(memory:NVMM) ! nvv4l2h264enc insert-sps-pps=true bitrate=%d ! h264parse ! rtph264pay name=pay0 pt=96 ! udpsink host=127.0.0.1 port=%d async=false", bitrate * 1000000, media_port);  // omxh264enc
#else
  sprintf(str_buf, "appsrc is-live=true ! videoconvert ! x264enc bitrate=%d ! video/x-h264, stream-format=byte-stream ! rtph264pay name=pay0 pt=96 ! udpsink host=127.0.0.1 port=%d async=false", bitrate * 1000000, media_port);
#endif

  std::string str(str_buf);
  this->_stream_writer = cv::VideoWriter(str, cv::CAP_GSTREAMER, 0, 30, this->_stream_size, true);

  return true;
#endif
  return false;
}

bool VideoStreamerGstreamerImpl::gstreamerIsOpened()
{
#ifdef WITH_GSTREAMER
  return this->_stream_writer.isOpened();
#endif
  return false;
}

void VideoStreamerGstreamerImpl::gstreamerWrite(cv::Mat img_)
{
#ifdef WITH_GSTREAMER
  this->_stream_writer.write(img_);
#endif
}

void VideoStreamerGstreamerImpl::gstreamerRelease()
{
#ifdef WITH_GSTREAMER
  if (this->_stream_writer.isOpened())
    this->_stream_writer.release();
#endif
}


}

