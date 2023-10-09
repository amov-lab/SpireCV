#include "veri_det_cuda_impl.h"
#include <cmath>
#include <fstream>

#define SV_MODEL_DIR "/SpireCV/models/"
#define SV_ROOT_DIR "/SpireCV/"

#ifdef WITH_CUDA
#include "yolov7/logging.h"
#define TRTCHECK(status)                                 \
  do                                                     \
  {                                                      \
    auto ret = (status);                                 \
    if (ret != 0)                                        \
    {                                                    \
      std::cerr << "Cuda failure: " << ret << std::endl; \
      abort();                                           \
    }                                                    \
  } while (0)

#define DEVICE 0 // GPU id
#define BATCH_SIZE 1

#define MAX_IMAGE_INPUT_SIZE_THRESH 3000 * 3000 // ensure it exceed the maximum size in the input images !
#endif

#include <iostream>
#include <cmath>
int BAT = 1;
float cosineSimilarity(float *vec1, float *vec2, int size)
{
  // 计算向量的点积
  float dotProduct = 0.0f;
  for (int i = 0; i < size; ++i)
  {
    dotProduct += vec1[i] * vec2[i];
  }

  // 计算向量的模长
  float magnitudeVec1 = 0.0f;
  float magnitudeVec2 = 0.0f;
  for (int i = 0; i < size; ++i)
  {
    magnitudeVec1 += vec1[i] * vec1[i];
    magnitudeVec2 += vec2[i] * vec2[i];
  }
  magnitudeVec1 = std::sqrt(magnitudeVec1);
  magnitudeVec2 = std::sqrt(magnitudeVec2);

  // 计算余弦相似性
  float similarity = dotProduct / (magnitudeVec1 * magnitudeVec2);

  return similarity;
}

namespace sv
{

  using namespace cv;

#ifdef WITH_CUDA
  using namespace nvinfer1;
  static Logger g_nvlogger;
#endif

  VeriDetectorCUDAImpl::VeriDetectorCUDAImpl()
  {
  }

  VeriDetectorCUDAImpl::~VeriDetectorCUDAImpl()
  {
  }

  bool VeriDetectorCUDAImpl::cudaSetup()
  {
#ifdef WITH_CUDA
    std::string trt_model_fn = get_home() + SV_MODEL_DIR + "model.engine";
    if (!is_file_exist(trt_model_fn))
    {
      throw std::runtime_error("SpireCV (104) Error loading the VERI TensorRT model (File Not Exist)");
    }
    char *trt_model_stream{nullptr};
    size_t trt_model_size{0};
    try
    {
      std::ifstream file(trt_model_fn, std::ios::binary);
      file.seekg(0, file.end);
      trt_model_size = file.tellg();
      file.seekg(0, file.beg);
      trt_model_stream = new char[trt_model_size];
      assert(trt_model_stream);
      file.read(trt_model_stream, trt_model_size);
      file.close();
    }
    catch (const std::runtime_error &e)
    {
      throw std::runtime_error("SpireCV (104) Error loading the TensorRT model!");
    }

    // TensorRT
    IRuntime *runtime = nvinfer1::createInferRuntime(g_nvlogger);
    assert(runtime != nullptr);
    ICudaEngine *p_cu_engine = runtime->deserializeCudaEngine(trt_model_stream, trt_model_size);
    assert(p_cu_engine != nullptr);
    this->_trt_context = p_cu_engine->createExecutionContext();
    assert(this->_trt_context != nullptr);

    delete[] trt_model_stream;
    const ICudaEngine &cu_engine = this->_trt_context->getEngine();
    assert(cu_engine.getNbBindings() == 2);

    this->_input_index = cu_engine.getBindingIndex("input");
    this->_output_index1 = cu_engine.getBindingIndex("output");
    this->_output_index2 = cu_engine.getBindingIndex("/head/layers.0/act/Mul_output_0");
    TRTCHECK(cudaMalloc(&_p_buffers[this->_input_index], 2 * 3 * 224 * 224 * sizeof(float)));
    TRTCHECK(cudaMalloc(&_p_buffers[this->_output_index1], 2 * 576 * sizeof(float)));
    TRTCHECK(cudaMalloc(&_p_buffers[this->_output_index2], 2 * 1280 * sizeof(float)));
    TRTCHECK(cudaStreamCreate(&_cu_stream));

    auto input_dims = nvinfer1::Dims4{2, 3, 224, 224};
    this->_trt_context->setBindingDimensions(this->_input_index, input_dims);

    this->_p_data = new float[2 * 3 * 224 * 224];
    this->_p_prob1 = new float[2 * 576];
    this->_p_prob2 = new float[2 * 1280];
    this->_p_prob3 = new float[2 * 1280];
    // Input
    TRTCHECK(cudaMemcpyAsync(_p_buffers[this->_input_index], this->_p_data, 2 * 3 * 224 * 224 * sizeof(float), cudaMemcpyHostToDevice, this->_cu_stream));
    // this->_trt_context->enqueue(1, _p_buffers, this->_cu_stream, nullptr);
    this->_trt_context->enqueueV2(_p_buffers, this->_cu_stream, nullptr);
    // Output
    TRTCHECK(cudaMemcpyAsync(this->_p_prob1, _p_buffers[this->_output_index1], 2 * 576 * sizeof(float), cudaMemcpyDeviceToHost, this->_cu_stream));
    TRTCHECK(cudaMemcpyAsync(this->_p_prob2, _p_buffers[this->_output_index2], 2 * 1280 * sizeof(float), cudaMemcpyDeviceToHost, this->_cu_stream));
    cudaStreamSynchronize(this->_cu_stream);
    return true;
#endif
    return false;
  }

  void VeriDetectorCUDAImpl::cudaRoiCNN(
      std::vector<cv::Mat> &input_rois_,
      std::vector<int> &output_labels_)
  {
#ifdef WITH_CUDA

    for (int i = 0; i < 2; ++i)
    {
      for (int row = 0; row < 224; ++row)
      {
        uchar *uc_pixel = input_rois_[i].data + row * input_rois_[i].step; // compute row id
        for (int col = 0; col < 224; ++col)
        {
          // mean=[136.20, 141.50, 145.41], std=[44.77, 44.20, 44.30]
          this->_p_data[224 * 224 * 3 * i + col + row * 224] = ((float)uc_pixel[0] - 136.20f) / 44.77f;
          this->_p_data[224 * 224 * 3 * i + col + row * 224 + 224 * 224] = ((float)uc_pixel[1] - 141.50f) / 44.20f;
          this->_p_data[224 * 224 * 3 * i + col + row * 224 + 224 * 224 * 2] = ((float)uc_pixel[2] - 145.41f) / 44.30f;
          uc_pixel += 3;
        }
      }
    }

    // Input
    TRTCHECK(cudaMemcpyAsync(_p_buffers[this->_input_index], this->_p_data, 2 * 3 * 224 * 224 * sizeof(float), cudaMemcpyHostToDevice, this->_cu_stream));
    // this->_trt_context->enqueue(1, _p_buffers, this->_cu_stream, nullptr);
    this->_trt_context->enqueueV2(_p_buffers, this->_cu_stream, nullptr);
    // Output
    TRTCHECK(cudaMemcpyAsync(this->_p_prob1, _p_buffers[this->_output_index1], 2 * 576 * sizeof(float), cudaMemcpyDeviceToHost, this->_cu_stream));
    TRTCHECK(cudaMemcpyAsync(this->_p_prob2, _p_buffers[this->_output_index2], 2 * 1280 * sizeof(float), cudaMemcpyDeviceToHost, this->_cu_stream));
    cudaStreamSynchronize(this->_cu_stream);

    // Find max index
    double max = 0;
    int label = 0;
    for (int i = 0; i < 576; ++i)
    {
      if (max < this->_p_prob1[i])
      {
        max = this->_p_prob1[i];
        label = i;
      }
    }

    // 计算两个数组的余弦相似性
    float similarity = cosineSimilarity(_p_prob2, _p_prob2 + 1280, 1280);
    std::cout << "余弦相似性: " << similarity << std::endl;
    std::cout << "VERI LABEL: " << label << std::endl;
  }
#endif
}
