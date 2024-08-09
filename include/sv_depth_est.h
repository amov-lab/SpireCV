#ifndef __SV_DEPTH_EST__
#define __SV_DEPTH_EST__

#include "sv_core.h"
#include <opencv2/opencv.hpp>
#include <opencv2/aruco.hpp>
#include <opencv2/tracking.hpp>
#include <string>
#include <chrono>


namespace sv {

class DepthEstimationCUDAImpl;

class MonocularDepthEstimation : public MonocularDepthEstimationBase
{
public:
  MonocularDepthEstimation();
  ~MonocularDepthEstimation();
protected:
  void _load();
  bool setupImpl();
  void predictImpl(
    cv::Mat img_,
    cv::Mat& mde_
  );

  DepthEstimationCUDAImpl *_cuda_impl;
};
}
#endif
