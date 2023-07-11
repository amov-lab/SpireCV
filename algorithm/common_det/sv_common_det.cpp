#include "sv_common_det.h"
#include <cmath>
#include <fstream>

#ifdef WITH_CUDA
#include <NvInfer.h>
#include <cuda_runtime_api.h>
#include "common_det_cuda_impl.h"
#endif


namespace sv {


CommonObjectDetector::CommonObjectDetector()
{
#ifdef WITH_CUDA
  this->_cuda_impl = new CommonObjectDetectorCUDAImpl;
#endif
}
CommonObjectDetector::~CommonObjectDetector()
{
}

bool CommonObjectDetector::setupImpl()
{
#ifdef WITH_CUDA
  return this->_cuda_impl->cudaSetup(this);
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
    boxes_seg_
  );
#endif
}





}

