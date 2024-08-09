#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <tuple>
#include <algorithm>
#include <opencv2/opencv.hpp>
#include "utils.h"
#include "sv_core.h"

#ifdef WITH_CUDA
#include <NvInfer.h>
#include <cuda_runtime_api.h>
#endif

namespace sv
{
class DepthEstimationCUDAImpl
{
public:
	DepthEstimationCUDAImpl();
	~DepthEstimationCUDAImpl();

	void predict(cv::Mat& img_, cv::Mat& mde_);
	bool cudaSetup(MonocularDepthEstimationBase* base_);

private:
	int input_w = 406;
	int input_h = 406;
	float mean[3] = { 123.675, 116.28, 103.53 };
	float std[3] = { 58.395, 57.12, 57.375 };

	std::vector<int> offset;

	nvinfer1::IRuntime* runtime;
	nvinfer1::ICudaEngine* engine;
	nvinfer1::IExecutionContext* context;

	void* buffer[2];
	float* depth_data;
	cudaStream_t stream;

	std::vector<float> preprocess(cv::Mat& img_);
};
}