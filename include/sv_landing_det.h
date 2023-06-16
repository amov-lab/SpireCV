#ifndef __SV_LANDING_DET__
#define __SV_LANDING_DET__

#include "sv_core.h"
#include <opencv2/opencv.hpp>
#include <opencv2/aruco.hpp>
#include <opencv2/tracking.hpp>
#include <string>
#include <chrono>


namespace sv {

class LandingMarkerDetectorCUDAImpl;

class LandingMarkerDetector : public LandingMarkerDetectorBase
{
public:
  LandingMarkerDetector();
  ~LandingMarkerDetector();
protected:
  bool setupImpl();
  void roiCNN(
    std::vector<cv::Mat>& input_rois_,
    std::vector<int>& output_labels_
  );

  LandingMarkerDetectorCUDAImpl* _cuda_impl;
};


}
#endif
