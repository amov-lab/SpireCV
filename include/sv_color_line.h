#ifndef __SV_COLOR_LINE__
#define __SV_COLOR_LINE__

#include "sv_core.h"
#include <opencv2/opencv.hpp>
#include <string>
#include <chrono>

namespace sv
{


class ColorLineDetector : public CameraAlgorithm
{
public:
  ColorLineDetector();
  ~ColorLineDetector();

  void detect(cv::Mat img_, TargetsInFrame &tgts_);

  cv::Point3d pose;

  double line_location;
  double line_location_a1;
  double line_location_a2;

  bool is_load_parameter;

  std::string line_color;

protected:
  float _cy_a1;
  float _cy_a2;
  float _half_h;
  float _half_w;

  void _load();
  float cnt_area(std::vector<cv::Point> cnt_);
  void get_line_area(cv::Mat &frame_, cv::Mat &line_area_, cv::Mat &line_area_a1_, cv::Mat &line_area_a2_);
  void seg(cv::Mat line_area_, cv::Mat line_area_a1_, cv::Mat line_area_a2_, std::string line_color_, cv::Point &center_, int &area_, cv::Point &center_a1_, cv::Point &center_a2_);
};
}
#endif