#ifndef __SV_TRACKING__
#define __SV_TRACKING__

#include "sv_core.h"
#include <opencv2/opencv.hpp>
#include <opencv2/aruco.hpp>
#include <opencv2/tracking.hpp>
#include <string>
#include <chrono>



namespace sv {

class SingleObjectTrackerOCV470Impl;

class SingleObjectTracker : public SingleObjectTrackerBase
{
public:
  SingleObjectTracker();
  ~SingleObjectTracker();
protected:
  bool setupImpl();
  void initImpl(cv::Mat img_, const cv::Rect& bounding_box_);
  bool trackImpl(cv::Mat img_, cv::Rect& output_bbox_);

  SingleObjectTrackerOCV470Impl* _ocv470_impl;
};


}
#endif
