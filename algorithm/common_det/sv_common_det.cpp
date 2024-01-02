#include "sv_common_det.h"
#include <cmath>
#include <fstream>

#ifdef WITH_CUDA
#include <NvInfer.h>
#include <cuda_runtime_api.h>
#include "common_det_cuda_impl.h"
#endif

#ifdef WITH_INTEL
#include <openvino/openvino.hpp>
#include "common_det_intel_impl.h"
#endif

namespace sv {


CommonObjectDetector::CommonObjectDetector(bool input_4k)
{
  this->_input_4k = input_4k;
#ifdef WITH_CUDA
  this->_cuda_impl = new CommonObjectDetectorCUDAImpl;
#endif

#ifdef WITH_INTEL
  this->_intel_impl = new CommonObjectDetectorIntelImpl;
#endif
}
CommonObjectDetector::~CommonObjectDetector()
{
}

bool CommonObjectDetector::setupImpl()
{
#ifdef WITH_CUDA
  return this->_cuda_impl->cudaSetup(this, this->_input_4k);
#endif

#ifdef WITH_INTEL
  return this->_intel_impl->intelSetup(this, this->_input_4k);
#endif
  return false;
}

void CommonObjectDetector::detectImpl(
  cv::Mat img_,
  std::vector<float>& boxes_x_,
  std::vector<float>& boxes_y_,
  std::vector<float>& boxes_w_,
  std::vector<float>& boxes_h_,
  std::vector<int>& boxes_label_,
  std::vector<float>& boxes_score_,
  std::vector<cv::Mat>& boxes_seg_
)
{
#ifdef WITH_CUDA
  this->_cuda_impl->cudaDetect(
    this,
    img_,
    boxes_x_,
    boxes_y_,
    boxes_w_,
    boxes_h_,
    boxes_label_,
    boxes_score_,
    boxes_seg_,
    this->_input_4k);
#endif

#ifdef WITH_INTEL
  this->_intel_impl->intelDetect(
    this,
    img_,
    boxes_x_,
    boxes_y_,
    boxes_w_,
    boxes_h_,
    boxes_label_,
    boxes_score_,
    boxes_seg_,
    this->_input_4k);
#endif
}

}

