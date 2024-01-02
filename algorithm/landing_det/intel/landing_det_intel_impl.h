#ifndef __SV_LANDING_DET_INTEL__
#define __SV_LANDING_DET_INTEL__

#include "sv_core.h"
#include <opencv2/opencv.hpp>
#include <opencv2/aruco.hpp>
#include <opencv2/tracking.hpp>
#include <string>
#include <chrono>

#ifdef WITH_INTEL
#include <openvino/openvino.hpp>
#endif

namespace sv
{
  class LandingMarkerDetectorIntelImpl
  {
  public:
    LandingMarkerDetectorIntelImpl();
    ~LandingMarkerDetectorIntelImpl();

    bool intelSetup();
    void intelRoiCNN(
        std::vector<cv::Mat> &input_rois_,
        std::vector<int> &output_labels_);

#ifdef WITH_INTEL
    float *_p_prob;

    ov::Tensor input_tensor;
    ov::InferRequest infer_request;
    ov::CompiledModel compiled_model;
#endif
  };
}
#endif
