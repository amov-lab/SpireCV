#include "sv_veri_det.h"
#include <cmath>
#include <fstream>
#include "gason.h"
#include "sv_util.h"

#ifdef WITH_CUDA
#include <NvInfer.h>
#include <cuda_runtime_api.h>
#include "veri_det_cuda_impl.h"
#endif

#define SV_ROOT_DIR "/SpireCV/"

namespace sv
{

  VeriDetector::VeriDetector()
  {
#ifdef WITH_CUDA
    this->_cuda_impl = new VeriDetectorCUDAImpl;
#endif
  }
  VeriDetector::~VeriDetector()
  {
  }

  void VeriDetector::_load()
  {
    JsonValue all_value;
    JsonAllocator allocator;
    _load_all_json(this->alg_params_fn, all_value, allocator);

    JsonValue veriliner_params_value;
    _parser_algorithm_params("VeriDetector", all_value, veriliner_params_value);

    for (auto i : veriliner_params_value)
    {
      if ("vehicle_ID" == std::string(i->key))
      {
        this->vehicle_id = i->value.toString();
        std::cout << "vehicle_ID Load Sucess!" << std::endl;
      }
    }
  }

  bool VeriDetector::setupImpl()
  {
#ifdef WITH_CUDA
    return this->_cuda_impl->cudaSetup();
#endif
  
    return false;
  }


  void VeriDetector::roiCNN(
      std::vector<cv::Mat> &input_rois_,
      std::vector<float> &output_labels_)
  {
#ifdef WITH_CUDA
    this->_cuda_impl->cudaRoiCNN(
        input_rois_,
        output_labels_);
#endif
  }

  void VeriDetector::detect(cv::Mat img_, const cv::Rect &bounding_box_, sv::Target &tgt)
  {
    if (!_params_loaded)
    {
      this->_load();
      this->_loadLabels();
      _params_loaded = true;
    }

    // convert Rect2d from left-up to center.
    targetPos[0] = float(bounding_box_.x) + float(bounding_box_.width) * 0.5f;
    targetPos[1] = float(bounding_box_.y) + float(bounding_box_.height) * 0.5f;

    targetSz[0] = float(bounding_box_.width);
    targetSz[1] = float(bounding_box_.height);

    // Extent the bounding box.
    float sumSz = targetSz[0] + targetSz[1];
    float wExtent = targetSz[0] + 0.5 * (sumSz);
    float hExtent = targetSz[1] + 0.5 * (sumSz);
    int sz = int(cv::sqrt(wExtent * hExtent));

    cv::Mat crop;
    getSubwindow(crop, img_, sz, 224);

    std::string img_ground_dir = get_home()  + SV_ROOT_DIR + this->vehicle_id;
    cv::Mat img_ground = cv::imread(img_ground_dir);
    cv::resize(img_ground, img_ground, cv::Size(224, 224));
    std::vector<cv::Mat> input_rois_ = {crop, img_ground};

#ifdef WITH_CUDA
    std::vector<float> output_labels;
    roiCNN(input_rois_, output_labels);

    // auto t1 = std::chrono::system_clock::now();
    // tgts_.setFPS(1000.0 / std::chrono::duration_cast<std::chrono::milliseconds>(t1 - this->_t0).count());
    // this->_t0 = std::chrono::system_clock::now();
    // tgts_.setTimeNow();

    if (output_labels.size() > 0)
    {
      // tgt.category_id = output_labels[0];
      tgt.sim_score = output_labels[1];
      // tgts_.targets.push_back(tgt);
    }
#endif
  }

    void VeriDetector::getSubwindow(cv::Mat &dstCrop, cv::Mat &srcImg, int originalSz, int resizeSz)
  {
    cv::Scalar avgChans = mean(srcImg);
    cv::Size imgSz = srcImg.size();
    int c = (originalSz + 1) / 2;

    int context_xmin = (int)(targetPos[0]) - c;
    int context_xmax = context_xmin + originalSz - 1;
    int context_ymin = (int)(targetPos[1]) - c;
    int context_ymax = context_ymin + originalSz - 1;

    int left_pad = std::max(0, -context_xmin);
    int top_pad = std::max(0, -context_ymin);
    int right_pad = std::max(0, context_xmax - imgSz.width + 1);
    int bottom_pad = std::max(0, context_ymax - imgSz.height + 1);

    context_xmin += left_pad;
    context_xmax += left_pad;
    context_ymin += top_pad;
    context_ymax += top_pad;

    cv::Mat cropImg;
    if (left_pad == 0 && top_pad == 0 && right_pad == 0 && bottom_pad == 0)
    {
      // Crop image without padding.
      cropImg = srcImg(cv::Rect(context_xmin, context_ymin,
                                context_xmax - context_xmin + 1, context_ymax - context_ymin + 1));
    }
    else // Crop image with padding, and the padding value is avgChans
    {
      cv::Mat tmpMat;
      cv::copyMakeBorder(srcImg, tmpMat, top_pad, bottom_pad, left_pad, right_pad, cv::BORDER_CONSTANT, avgChans);
      cropImg = tmpMat(cv::Rect(context_xmin, context_ymin, context_xmax - context_xmin + 1, context_ymax - context_ymin + 1));
    }
    resize(cropImg, dstCrop, cv::Size(resizeSz, resizeSz));
  }

}
