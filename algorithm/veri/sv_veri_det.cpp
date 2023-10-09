#include "sv_veri_det.h"
#include <cmath>
#include <fstream>
#ifdef WITH_CUDA
#include <NvInfer.h>
#include <cuda_runtime_api.h>
#include "veri_det_cuda_impl.h"
#endif


namespace sv {


VeriDetector::VeriDetector()
{
  this->_cuda_impl = new VeriDetectorCUDAImpl;
}
VeriDetector::~VeriDetector()
{
}

bool VeriDetector::setupImpl()
{
#ifdef WITH_CUDA
  return this->_cuda_impl->cudaSetup();
#endif
  return false;
}

void VeriDetector::roiCNN(
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
}



void VeriDetector::detect(cv::Mat img1_, cv::Mat img2_, TargetsInFrame& tgts_)
{
  if (!_params_loaded)
  {
    this->_load();
    this->_loadLabels();
    _params_loaded = true;
  }

  std::vector<cv::Mat> e_roi = {img1_, img2_};

  std::vector<int> output_labels;
  roiCNN(e_roi, output_labels);
}

}

