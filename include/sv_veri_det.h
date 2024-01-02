#ifndef __SV_VERI_DET__
#define __SV_VERI_DET__

#include "sv_core.h"
#include <opencv2/opencv.hpp>
#include <opencv2/aruco.hpp>
#include <opencv2/tracking.hpp>
#include <string>
#include <chrono>

namespace sv
{

class VeriDetectorCUDAImpl;
class VeriDetectorIntelImpl;

class VeriDetector : public LandingMarkerDetectorBase
{
public:
  VeriDetector();
  ~VeriDetector();

  void detect(cv::Mat img_, const cv::Rect &bounding_box_, sv::Target &tgt);

protected:
  void _load();
  bool setupImpl();
  void roiCNN(
    std::vector<cv::Mat> &input_rois_,
    std::vector<float> &output_labels_);
  void getSubwindow(cv::Mat &dstCrop, cv::Mat &srcImg, int originalSz, int resizeSz);

  std::string vehicle_id;
    
  // Save the target bounding box for each frame.
  std::vector<float> targetSz = {0, 0};  // H and W of bounding box
  std::vector<float> targetPos = {0, 0}; // center point of bounding box (x, y)

  VeriDetectorCUDAImpl *_cuda_impl;
  VeriDetectorIntelImpl *_intel_impl;
};

}
#endif
