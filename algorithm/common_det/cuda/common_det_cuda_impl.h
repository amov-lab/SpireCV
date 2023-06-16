#ifndef __SV_COMMON_DET_CUDA__
#define __SV_COMMON_DET_CUDA__

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


class CommonObjectDetectorCUDAImpl
{
public:
  CommonObjectDetectorCUDAImpl();
  ~CommonObjectDetectorCUDAImpl();

  bool cudaSetup(CommonObjectDetectorBase* base_);
  void cudaDetect(
    CommonObjectDetectorBase* base_,
    cv::Mat img_,
    std::vector<float>& boxes_x_,
    std::vector<float>& boxes_y_,
    std::vector<float>& boxes_w_,
    std::vector<float>& boxes_h_,
    std::vector<int>& boxes_label_,
    std::vector<float>& boxes_score_,
    std::vector<cv::Mat>& boxes_seg_
  );

#ifdef WITH_CUDA
  void _prepare_buffers_seg(int input_h, int input_w);
  void _prepare_buffers(int input_h, int input_w);
  nvinfer1::IExecutionContext* _context;
  nvinfer1::IRuntime* _runtime;
  nvinfer1::ICudaEngine* _engine;
  cudaStream_t _stream;
  float* _gpu_buffers[3];
  float* _cpu_output_buffer;
  float* _cpu_output_buffer1;
  float* _cpu_output_buffer2;
#endif
};


}
#endif
