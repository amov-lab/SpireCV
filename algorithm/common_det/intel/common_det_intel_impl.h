#ifndef __SV_COMMON_DET_INTEL__
#define __SV_COMMON_DET_INTEL__

#include "sv_core.h"
#include <string>
#include <iostream>
#include <opencv2/dnn.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <string>
#include <chrono>

#ifdef WITH_INTEL
#include <openvino/openvino.hpp>
#endif

struct Resize
{
  cv::Mat resized_image;
  int dw;
  int dh;
};

struct Detection
{
  int class_id;
  float confidence;
  cv::Rect box;
};

namespace sv
{

  class CommonObjectDetectorIntelImpl
  {
  public:
    CommonObjectDetectorIntelImpl();
    ~CommonObjectDetectorIntelImpl();

    bool intelSetup(CommonObjectDetectorBase *base_, bool input_4k_);
    void intelDetect(
        CommonObjectDetectorBase *base_,
        cv::Mat img_,
        std::vector<float> &boxes_x_,
        std::vector<float> &boxes_y_,
        std::vector<float> &boxes_w_,
        std::vector<float> &boxes_h_,
        std::vector<int> &boxes_label_,
        std::vector<float> &boxes_score_,
        std::vector<cv::Mat> &boxes_seg_,
        bool input_4k_);
    void preprocess_img(cv::Mat &img_);
    void preprocess_img_seg(cv::Mat &img_, std::vector<float> &paddings);
    void postprocess_img_seg(cv::Mat &img_,
                             std::vector<float> &paddings,
                             std::vector<float> &boxes_x_,
                             std::vector<float> &boxes_y_,
                             std::vector<float> &boxes_w_,
                             std::vector<float> &boxes_h_,
                             std::vector<int> &boxes_label_,
                             std::vector<float> &boxes_score_,
                             std::vector<cv::Mat> &boxes_seg_,
                             double &thrs_conf,
                             double &thrs_nms);

    void postprocess_img(std::vector<float> &boxes_x_,
                         std::vector<float> &boxes_y_,
                         std::vector<float> &boxes_w_,
                         std::vector<float> &boxes_h_,
                         std::vector<int> &boxes_label_,
                         std::vector<float> &boxes_score_,
                         double &thrs_conf,
                         double &thrs_nms);

#ifdef WITH_INTEL
    int inpWidth;
    int inpHeight;
    bool with_segmentation;
    float rx; // the width ratio of original image and resized image
    float ry; // the height ratio of original image and resized image
    Resize resize;
    ov::Tensor input_tensor;
    ov::InferRequest infer_request;
    ov::CompiledModel compiled_model;
#endif
  };

}
#endif
