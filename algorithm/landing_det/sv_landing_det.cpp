#include "sv_landing_det.h"
#include <cmath>
#include <fstream>
#ifdef WITH_CUDA
#include <NvInfer.h>
#include <cuda_runtime_api.h>
#include "landing_det_cuda_impl.h"
#endif

#ifdef WITH_INTEL
#include <openvino/openvino.hpp>
#include "landing_det_intel_impl.h"
#endif

namespace sv {


LandingMarkerDetector::LandingMarkerDetector()
{
#ifdef WITH_CUDA
  this->_cuda_impl = new LandingMarkerDetectorCUDAImpl;
#endif

#ifdef WITH_INTEL
  this->_intel_impl = new LandingMarkerDetectorIntelImpl;
#endif
}
LandingMarkerDetector::~LandingMarkerDetector()
{
}

bool LandingMarkerDetector::setupImpl()
{
#ifdef WITH_CUDA
  return this->_cuda_impl->cudaSetup();
#endif

#ifdef WITH_INTEL
  return this->_intel_impl->intelSetup();
#endif
  return false;
}

void LandingMarkerDetector::roiCNN(
  std::vector<cv::Mat>& input_rois_,
  std::vector<int>& output_labels_
)
{
#ifdef WITH_CUDA
  this->_cuda_impl->cudaRoiCNN(
    input_rois_,
    output_labels_
  );
#endif

#ifdef WITH_INTEL
  this->_intel_impl->intelRoiCNN(
    input_rois_,
    output_labels_
  );
#endif
}
}

