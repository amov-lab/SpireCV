#include "veri_det_intel_impl.h"
#include <cmath>
#include <fstream>

#define SV_MODEL_DIR "/SpireCV/models/"
#define SV_ROOT_DIR "/SpireCV/"

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
#ifdef WITH_INTEL
  using namespace cv;
  using namespace std;
  using namespace dnn;
#endif

  VeriDetectorIntelImpl::VeriDetectorIntelImpl()
  {
  }

  VeriDetectorIntelImpl::~VeriDetectorIntelImpl()
  {
  }

  bool VeriDetectorIntelImpl::intelSetup()
  {
#ifdef WITH_INTEL
    std::string onnx_model_fn = get_home() + SV_MODEL_DIR + "veri.onnx";
    if (!is_file_exist(onnx_model_fn))
    {
      throw std::runtime_error("SpireCV (104) Error loading the VeriDetector openVINO model (File Not Exist)");
    }

    // OpenVINO
    ov::Core core;
    this->compiled_model = core.compile_model(onnx_model_fn, "GPU");
    this->infer_request = compiled_model.create_infer_request();

    return true;
#endif
    return false;
  }

  void VeriDetectorIntelImpl::intelRoiCNN(
      std::vector<cv::Mat> &input_rois_,
      std::vector<float> &output_labels_)
  {
#ifdef WITH_INTEL

    Mat blobs;
    blobFromImages(input_rois_, blobs, 1 / 255.0, Size(224, 224), Scalar(0, 0, 0), true, true);

    auto input_port = compiled_model.input();
    ov::Tensor input_tensor(input_port.get_element_type(), input_port.get_shape(), blobs.ptr(0));

    infer_request.infer();

    const ov::Tensor &label_pre = infer_request.get_output_tensor(0);
    this->_p_prob1 = label_pre.data<float>();

    const ov::Tensor &proto_pre = infer_request.get_output_tensor(1);
    this->_p_prob2 = proto_pre.data<float>();

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

    float similarity = cosineSimilarity(this->_p_prob2, this->_p_prob2 + 1280, 1280);
    
    output_labels_.push_back(label);
    output_labels_.push_back(similarity);
#endif
  }


}
