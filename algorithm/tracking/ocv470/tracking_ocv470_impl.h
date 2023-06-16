#ifndef __SV_TRACKING_OCV470__
#define __SV_TRACKING_OCV470__

#include "sv_core.h"
#include <opencv2/opencv.hpp>
#include <opencv2/aruco.hpp>
#include <opencv2/tracking.hpp>
#include <string>
#include <chrono>



namespace sv {


class SingleObjectTrackerOCV470Impl
{
public:
  SingleObjectTrackerOCV470Impl();
  ~SingleObjectTrackerOCV470Impl();

  bool ocv470Setup(SingleObjectTrackerBase* base_);
  void ocv470Init(cv::Mat img_, const cv::Rect& bounding_box_);
  bool ocv470Track(cv::Mat img_, cv::Rect& output_bbox_);

  std::string _algorithm;
  int _backend;
  int _target;

#ifdef WITH_OCV470
  cv::Ptr<cv::TrackerDaSiamRPN> _siam_rpn;
  cv::Ptr<cv::TrackerKCF> _kcf;
  cv::Ptr<cv::TrackerCSRT> _csrt;
  cv::Ptr<cv::TrackerNano> _nano;
#endif
};


}
#endif
