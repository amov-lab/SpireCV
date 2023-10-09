#ifndef __SV_VERI_DET__
#define __SV_VERI_DET__

#include "sv_core.h"
#include <opencv2/opencv.hpp>
#include <opencv2/aruco.hpp>
#include <opencv2/tracking.hpp>
#include <string>
#include <chrono>


namespace sv {

class VeriDetectorCUDAImpl;

class VeriDetector : public LandingMarkerDetectorBase
{
public:
  VeriDetector();
  ~VeriDetector();

  void detect(cv::Mat img1_, cv::Mat img2_, TargetsInFrame &tgts_);

protected:
  bool setupImpl();
  void roiCNN(
    std::vector<cv::Mat>& input_rois_,
    std::vector<int>& output_labels_
  );

  VeriDetectorCUDAImpl* _cuda_impl;
};


}
#endif
