#include "sv_depth_est.h"
#include <cmath>
#include <fstream>
#include "gason.h"
#include "sv_util.h"

#ifdef WITH_CUDA
#include <NvInfer.h>
#include <cuda_runtime_api.h>
#include "depth_est_cuda_impl.h"
#endif

#define SV_ROOT_DIR "/SpireCV/"

namespace sv
{

  MonocularDepthEstimation::MonocularDepthEstimation()
  {
#ifdef WITH_CUDA
    this->_cuda_impl = new DepthEstimationCUDAImpl;
#endif
  }
  MonocularDepthEstimation::~MonocularDepthEstimation()
  {
  }

  bool MonocularDepthEstimation::setupImpl()
  {
#ifdef WITH_CUDA
    return this->_cuda_impl->cudaSetup(this);
#endif
    return false;
  }

  void MonocularDepthEstimation::predictImpl(cv::Mat img_, cv::Mat& mde_)
  {
#ifdef WITH_CUDA
    this->_cuda_impl->predict(img_, mde_);
#endif
  }

}
