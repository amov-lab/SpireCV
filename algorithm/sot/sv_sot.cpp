#include "sv_sot.h"
#include <cmath>
#include <fstream>
#include "sot_ocv470_impl.h"


namespace sv {


SingleObjectTracker::SingleObjectTracker()
{
  this->_ocv470_impl = new SingleObjectTrackerOCV470Impl;
}
SingleObjectTracker::~SingleObjectTracker()
{
}

bool SingleObjectTracker::setupImpl()
{
#ifdef WITH_OCV470
  return this->_ocv470_impl->ocv470Setup(this);
#endif
  return false;
}
void SingleObjectTracker::initImpl(cv::Mat img_, const cv::Rect& bounding_box_)
{
#ifdef WITH_OCV470
  this->_ocv470_impl->ocv470Init(img_, bounding_box_);
#endif
}
bool SingleObjectTracker::trackImpl(cv::Mat img_, cv::Rect& output_bbox_)
{
#ifdef WITH_OCV470
  return this->_ocv470_impl->ocv470Track(img_, output_bbox_);
#endif
  return false;
}


}

