#ifndef __SV_LANDING_DET_CUDA__
#define __SV_LANDING_DET_CUDA__

#include "sv_core.h"
#include <opencv2/opencv.hpp>
#include <opencv2/aruco.hpp>
#include <opencv2/tracking.hpp>
#include <string>
#include <chrono>



#ifdef WITH_CUDA
#include <NvInfer.h>
#include <cuda_runtime_api.h>
#endif



namespace sv {


class LandingMarkerDetectorCUDAImpl
{
public:
  LandingMarkerDetectorCUDAImpl();
  ~LandingMarkerDetectorCUDAImpl();

  bool cudaSetup();
  void cudaRoiCNN(
    std::vector<cv::Mat>& input_rois_,
    std::vector<int>& output_labels_
  );

#ifdef WITH_CUDA
  float *_p_data;
  float *_p_prob;
  nvinfer1::IExecutionContext *_trt_context;
  int _input_index;
  int _output_index;
  void *_p_buffers[2];
  cudaStream_t _cu_stream;
#endif
};


}
#endif
