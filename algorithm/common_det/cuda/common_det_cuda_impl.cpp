#include "common_det_cuda_impl.h"
#include <cmath>
#include <fstream>

#define SV_MODEL_DIR "/SpireCV/models/"
#define SV_ROOT_DIR "/SpireCV/"


#ifdef WITH_CUDA
#include "yolov7/cuda_utils.h"
#include "yolov7/logging.h"
#include "yolov7/utils.h"
#include "yolov7/preprocess.h"
#include "yolov7/postprocess.h"
#include "yolov7/model.h"
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

#define DEVICE 0  // GPU id
#define BATCH_SIZE 1
#define MAX_IMAGE_INPUT_SIZE_THRESH 3000 * 3000 // ensure it exceed the maximum size in the input images !
#endif


namespace sv {

using namespace cv;


#ifdef WITH_CUDA
using namespace nvinfer1;
static Logger g_nvlogger;
const static int kOutputSize = kMaxNumOutputBbox * sizeof(Detection) / sizeof(float) + 1;
const static int kOutputSize1 = kMaxNumOutputBbox * sizeof(Detection) / sizeof(float) + 1;
const static int kOutputSize2 = 32 * (640 / 4) * (640 / 4);
#endif


CommonObjectDetectorCUDAImpl::CommonObjectDetectorCUDAImpl()
{
#ifdef WITH_CUDA
  this->_gpu_buffers[0] = nullptr;
  this->_gpu_buffers[1] = nullptr;
  this->_gpu_buffers[2] = nullptr;
  this->_cpu_output_buffer = nullptr;
  this->_cpu_output_buffer1 = nullptr;
  this->_cpu_output_buffer2 = nullptr;
  this->_context = nullptr;
  this->_engine = nullptr;
  this->_runtime = nullptr;
#endif
}


CommonObjectDetectorCUDAImpl::~CommonObjectDetectorCUDAImpl()
{
#ifdef WITH_CUDA
  // Release stream and buffers
  cudaStreamDestroy(_stream);
  if (_gpu_buffers[0])
    CUDA_CHECK(cudaFree(_gpu_buffers[0]));
  if (_gpu_buffers[1])
    CUDA_CHECK(cudaFree(_gpu_buffers[1]));
  if (_gpu_buffers[2])
    CUDA_CHECK(cudaFree(_gpu_buffers[2]));
  if (_cpu_output_buffer)
    delete[] _cpu_output_buffer;
  if (_cpu_output_buffer1)
    delete[] _cpu_output_buffer1;
  if (_cpu_output_buffer2)
    delete[] _cpu_output_buffer2;
  cuda_preprocess_destroy();
  // Destroy the engine
  if (_context)
    _context->destroy();
  if (_engine)
    _engine->destroy();
  if (_runtime)
    _runtime->destroy();
#endif
}


#ifdef WITH_CUDA
void infer(IExecutionContext& context, cudaStream_t& stream, void** gpu_buffers, float* output, int batchsize) {
  context.enqueue(batchsize, gpu_buffers, stream, nullptr);
  // context.enqueueV2(gpu_buffers, stream, nullptr);
  CUDA_CHECK(cudaMemcpyAsync(output, gpu_buffers[1], batchsize * kOutputSize * sizeof(float), cudaMemcpyDeviceToHost, stream));
  cudaStreamSynchronize(stream);
}
void infer_seg(IExecutionContext& context, cudaStream_t& stream, void **buffers, float* output1, float* output2, int batchSize) {
  context.enqueue(batchSize, buffers, stream, nullptr);
  // context.enqueueV2(buffers, stream, nullptr);
  CUDA_CHECK(cudaMemcpyAsync(output1, buffers[1], batchSize * kOutputSize1 * sizeof(float), cudaMemcpyDeviceToHost, stream));
  CUDA_CHECK(cudaMemcpyAsync(output2, buffers[2], batchSize * kOutputSize2 * sizeof(float), cudaMemcpyDeviceToHost, stream));
  cudaStreamSynchronize(stream);
}
void CommonObjectDetectorCUDAImpl::_prepare_buffers(int input_h, int input_w) {
  assert(this->_engine->getNbBindings() == 2);
  // In order to bind the buffers, we need to know the names of the input and output tensors.
  // Note that indices are guaranteed to be less than IEngine::getNbBindings()
  const int inputIndex = this->_engine->getBindingIndex(kInputTensorName);
  const int outputIndex = this->_engine->getBindingIndex(kOutputTensorName);
  assert(inputIndex == 0);
  assert(outputIndex == 1);
  // Create GPU buffers on device
  CUDA_CHECK(cudaMalloc((void**)&(this->_gpu_buffers[0]), kBatchSize * 3 * input_h * input_w * sizeof(float)));
  CUDA_CHECK(cudaMalloc((void**)&(this->_gpu_buffers[1]), kBatchSize * kOutputSize * sizeof(float)));

  this->_cpu_output_buffer = new float[kBatchSize * kOutputSize];
}
void CommonObjectDetectorCUDAImpl::_prepare_buffers_seg(int input_h, int input_w) {
  assert(this->_engine->getNbBindings() == 3);
  // In order to bind the buffers, we need to know the names of the input and output tensors.
  // Note that indices are guaranteed to be less than IEngine::getNbBindings()
  const int inputIndex = this->_engine->getBindingIndex(kInputTensorName);
  const int outputIndex1 = this->_engine->getBindingIndex(kOutputTensorName);
  const int outputIndex2 = this->_engine->getBindingIndex("proto");
  assert(inputIndex == 0);
  assert(outputIndex1 == 1);
  assert(outputIndex2 == 2);

  // Create GPU buffers on device
  CUDA_CHECK(cudaMalloc((void**)&(this->_gpu_buffers[0]), kBatchSize * 3 * input_h * input_w * sizeof(float)));
  CUDA_CHECK(cudaMalloc((void**)&(this->_gpu_buffers[1]), kBatchSize * kOutputSize1 * sizeof(float)));
  CUDA_CHECK(cudaMalloc((void**)&(this->_gpu_buffers[2]), kBatchSize * kOutputSize2 * sizeof(float)));

  // Alloc CPU buffers
  this->_cpu_output_buffer1 = new float[kBatchSize * kOutputSize1];
  this->_cpu_output_buffer2 = new float[kBatchSize * kOutputSize2];
}
void deserialize_engine(std::string& engine_name, IRuntime** runtime, ICudaEngine** engine, IExecutionContext** context) {
  std::ifstream file(engine_name, std::ios::binary);
  if (!file.good()) {
    std::cerr << "read " << engine_name << " error!" << std::endl;
    assert(false);
  }
  size_t size = 0;
  file.seekg(0, file.end);
  size = file.tellg();
  file.seekg(0, file.beg);
  char* serialized_engine = new char[size];
  assert(serialized_engine);
  file.read(serialized_engine, size);
  file.close();

  *runtime = createInferRuntime(g_nvlogger);
  assert(*runtime);
  *engine = (*runtime)->deserializeCudaEngine(serialized_engine, size);
  assert(*engine);
  *context = (*engine)->createExecutionContext();
  assert(*context);
  delete[] serialized_engine;
}
#endif


void CommonObjectDetectorCUDAImpl::cudaDetect(
  CommonObjectDetectorBase* base_,
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
  int input_h = base_->getInputH();
  int input_w = base_->getInputW();
  bool with_segmentation = base_->withSegmentation();
  double thrs_conf = base_->getThrsConf();
  double thrs_nms = base_->getThrsNms();

  std::vector<cv::Mat> img_batch;
  img_batch.push_back(img_);
  // Preprocess
  cuda_batch_preprocess(img_batch, this->_gpu_buffers[0], input_w, input_h, this->_stream);

  // Run inference
  if (with_segmentation)
  {
    infer_seg(*this->_context, this->_stream, (void**)this->_gpu_buffers, this->_cpu_output_buffer1, this->_cpu_output_buffer2, kBatchSize);
  }
  else
  {
    infer(*this->_context, this->_stream, (void**)this->_gpu_buffers, this->_cpu_output_buffer, kBatchSize);
  }

  // NMS
  std::vector<std::vector<Detection>> res_batch;
  if (with_segmentation)
  {
    batch_nms(res_batch, this->_cpu_output_buffer1, img_batch.size(), kOutputSize1, thrs_conf, thrs_nms);
  }
  else
  {
    batch_nms(res_batch, this->_cpu_output_buffer, img_batch.size(), kOutputSize, thrs_conf, thrs_nms);
  }

  std::vector<Detection> res = res_batch[0];
  std::vector<cv::Mat> masks;
  if (with_segmentation)
  {
    masks = process_mask(&(this->_cpu_output_buffer2[0]), kOutputSize2, res, input_h, input_w);
  }



  for (size_t j = 0; j < res.size(); j++) {
    cv::Rect r = get_rect(img_, res[j].bbox, input_h, input_w);
    if (r.x < 0) r.x = 0;
    if (r.y < 0) r.y = 0;
    if (r.x + r.width >= img_.cols)  r.width = img_.cols - r.x - 1;
    if (r.y + r.height >= img_.rows) r.height = img_.rows - r.y - 1;
    if (r.width > 5 && r.height > 5)
    {
      // cv::rectangle(img_show, r, cv::Scalar(0, 0, 255), 2);
      // cv::putText(img_show, vehiclenames[(int)res[j].class_id], cv::Point(r.x, r.y - 1), cv::FONT_HERSHEY_PLAIN, 2.2, cv::Scalar(0, 0, 255), 2);
      boxes_x_.push_back(r.x);
      boxes_y_.push_back(r.y);
      boxes_w_.push_back(r.width);
      boxes_h_.push_back(r.height);

      boxes_label_.push_back((int)res[j].class_id);
      boxes_score_.push_back(res[j].conf);

      if (with_segmentation)
      {
        cv::Mat mask_j = masks[j].clone();
        boxes_seg_.push_back(mask_j);
      }
    }
  }
  
#endif
}

bool CommonObjectDetectorCUDAImpl::cudaSetup(CommonObjectDetectorBase* base_)
{
#ifdef WITH_CUDA
  std::string dataset = base_->getDataset();
  int input_h = base_->getInputH();
  int input_w = base_->getInputW();
  bool with_segmentation = base_->withSegmentation();
  double thrs_conf = base_->getThrsConf();
  double thrs_nms = base_->getThrsNms();

  std::string engine_fn = get_home() + SV_MODEL_DIR + dataset + ".engine";
  if (input_w == 1280)
  {
    engine_fn = get_home() + SV_MODEL_DIR + dataset + "_HD.engine";
  }
  if (with_segmentation)
  {
    base_->setInputH(640);
    base_->setInputW(640);
    engine_fn = get_home() + SV_MODEL_DIR + dataset + "_SEG.engine";
  }
  std::cout << "Load: " << engine_fn << std::endl;
  if (!is_file_exist(engine_fn))
  {
    throw std::runtime_error("SpireCV (104) Error loading the CommonObject TensorRT model (File Not Exist)");
  }

  deserialize_engine(engine_fn, &this->_runtime, &this->_engine, &this->_context);
  CUDA_CHECK(cudaStreamCreate(&this->_stream));

  // Init CUDA preprocessing
  cuda_preprocess_init(kMaxInputImageSize);

  if (with_segmentation)
  {
    // Prepare cpu and gpu buffers
    this->_prepare_buffers_seg(input_h, input_w);
  }
  else
  {
    // Prepare cpu and gpu buffers
    this->_prepare_buffers(input_h, input_w);
  }
  return true;
#endif
  return false;
}













}

