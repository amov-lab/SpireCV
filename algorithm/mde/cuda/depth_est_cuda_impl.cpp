#include "depth_est_cuda_impl.h"
#include "yolov7/logging.h"
#include "sv_util.h"

#define SV_MODEL_DIR "/SpireCV/models/"
#define SV_ROOT_DIR "/SpireCV/"

namespace sv
{
#ifdef WITH_CUDA  
    using namespace nvinfer1;
    static Logger logger;
#endif

DepthEstimationCUDAImpl::DepthEstimationCUDAImpl()
{
}


DepthEstimationCUDAImpl::~DepthEstimationCUDAImpl()
{
    cudaFree(stream);
    cudaFree(buffer[0]);
    cudaFree(buffer[1]);

    delete[] depth_data;
}

bool DepthEstimationCUDAImpl::cudaSetup(MonocularDepthEstimationBase* base_)
{
#ifdef WITH_CUDA

    int indoors_or_outdoors = base_->getIndoorsOrOutdoors();
    std::string MODEL_NAME;
    if(indoors_or_outdoors == 0)
    {
        MODEL_NAME = "MonDepthEst_In.engine";
    }
    else{
        MODEL_NAME = "MonDepthEst_Out.engine";
    }
    std::string trt_model_fn = get_home() + SV_MODEL_DIR + MODEL_NAME;
    std::cout << "Load: " << trt_model_fn << std::endl;
    
    if (!is_file_exist(trt_model_fn))
    {
      throw std::runtime_error("SpireCV (104) Error loading the MonDepthEst TensorRT model (File Not Exist)");
    }
    // Deserialize an engine
    // read the engine file
    std::ifstream engineStream(trt_model_fn, std::ios::binary);
    engineStream.seekg(0, std::ios::end);
    const size_t modelSize = engineStream.tellg();
    engineStream.seekg(0, std::ios::beg);
    std::unique_ptr<char[]> engineData(new char[modelSize]);
    engineStream.read(engineData.get(), modelSize);
    engineStream.close();

    // create tensorrt model
    runtime = nvinfer1::createInferRuntime(logger);
    engine = runtime->deserializeCudaEngine(engineData.get(), modelSize);
    context = engine->createExecutionContext();


    // Define input dimensions
    auto input_dims = engine->getBindingDimensions(0);
    input_h = input_dims.d[2];
    input_w = input_dims.d[3];

    // create CUDA stream
    cudaStreamCreate(&stream);

    cudaMalloc(&buffer[0], 3 * input_h * input_w * sizeof(float));
    cudaMalloc(&buffer[1], input_h * input_w * sizeof(float));

    depth_data = new float[input_h * input_w];
    return true;
#endif
    return false;
}


std::vector<float> DepthEstimationCUDAImpl::preprocess(cv::Mat& img_)
{
    std::tuple<cv::Mat, int, int> resized = resize_depth(img_, input_w, input_h);
    cv::Mat resized_image = std::get<0>(resized);
    std::vector<float> input_tensor;
    for (int k = 0; k < 3; k++)
    {
        for (int i = 0; i < resized_image.rows; i++)
        {
            for (int j = 0; j < resized_image.cols; j++)
            {
                input_tensor.emplace_back(((float)resized_image.at<cv::Vec3b>(i, j)[k] - mean[k]) / std[k]);
            }
        }
    }
    return input_tensor;
}

void DepthEstimationCUDAImpl::predict(cv::Mat& img_, cv::Mat& mde_)
{
#ifdef WITH_CUDA
    cv::Mat clone_image;
    img_.copyTo(clone_image);

    int img_w = img_.cols;
    int img_h = img_.rows;

    // Preprocessing
    std::vector<float> input = preprocess(clone_image);
    cudaMemcpyAsync(buffer[0], input.data(), 3 * input_h * input_w * sizeof(float), cudaMemcpyHostToDevice, stream);

    // Inference depth model
    context->enqueueV2(buffer, stream, nullptr);
    cudaStreamSynchronize(stream);

    // Postprocessing
    cudaMemcpyAsync(depth_data, buffer[1], input_h * input_w * sizeof(float), cudaMemcpyDeviceToHost);

    // Convert the entire depth_data vector to a CV_32FC1 Mat
    cv::Mat depth_mat(input_h, input_w, CV_32FC1, depth_data);

    depth_mat.copyTo(mde_);
#endif
}
}



