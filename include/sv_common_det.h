#ifndef __SV_COMMON_DET__
#define __SV_COMMON_DET__

#include "sv_core.h"
#include <opencv2/opencv.hpp>
#include <opencv2/aruco.hpp>
#include <opencv2/tracking.hpp>
#include <string>
#include <chrono>


namespace sv {

class CommonObjectDetectorCUDAImpl;

class CommonObjectDetector : public CommonObjectDetectorBase
{
public:
  CommonObjectDetector();
  ~CommonObjectDetector();
protected:
  bool setupImpl();
  void detectImpl(
    cv::Mat img_,
    std::vector<float>& boxes_x_,
    std::vector<float>& boxes_y_,
    std::vector<float>& boxes_w_,
    std::vector<float>& boxes_h_,
    std::vector<int>& boxes_label_,
    std::vector<float>& boxes_score_,
    std::vector<cv::Mat>& boxes_seg_
  );

  CommonObjectDetectorCUDAImpl* _cuda_impl;
};


}
#endif
