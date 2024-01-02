#ifndef __SV_VERI_DET_INTEL__
#define __SV_VERI_DET_INTEL__

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
  class VeriDetectorIntelImpl
  {
  public:
    VeriDetectorIntelImpl();
    ~VeriDetectorIntelImpl();

    bool intelSetup();
    void intelRoiCNN(
        std::vector<cv::Mat> &input_rois_,
        std::vector<float> &output_labels_);

#ifdef WITH_INTEL

    float *_p_data;
    float *_p_prob1;
    float *_p_prob2;

    ov::Tensor input_tensor;
    ov::InferRequest infer_request;
    ov::CompiledModel compiled_model;
#endif
  };

}
#endif
