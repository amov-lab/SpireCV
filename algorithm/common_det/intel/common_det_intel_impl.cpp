#include "common_det_intel_impl.h"
#include <cmath>
#include <fstream>
#include "sv_util.h"

#define SV_MODEL_DIR "/SpireCV/models/"
#define SV_ROOT_DIR "/SpireCV/"

namespace sv
{

#ifdef WITH_INTEL
  using namespace cv;
  using namespace std;
  using namespace dnn;
#endif

  float sigmoid_function(float a)
  {
    float b = 1. / (1. + exp(-a));
    return b;
  }

  cv::Mat letterbox(cv::Mat &img_, std::vector<float> &paddings)
  {
    std::vector<int> new_shape = {640, 640};

    // Get current image shape [height, width]
    int img_h = img_.rows;
    int img_w = img_.cols;

    // Compute scale ratio(new / old) and target resized shape
    float scale = std::min(new_shape[1] * 1.0 / img_h, new_shape[0] * 1.0 / img_w);
    int resize_h = int(round(img_h * scale));
    int resize_w = int(round(img_w * scale));
    paddings[0] = scale;

    // Compute padding
    int pad_h = new_shape[1] - resize_h;
    int pad_w = new_shape[0] - resize_w;

    // Resize and pad image while meeting stride-multiple constraints
    cv::Mat resized_img;
    cv::resize(img_, resized_img, cv::Size(resize_w, resize_h));

    // divide padding into 2 sides
    float half_h = pad_h * 1.0 / 2;
    float half_w = pad_w * 1.0 / 2;
    paddings[1] = half_h;
    paddings[2] = half_w;

    // Compute padding boarder
    int top = int(round(half_h - 0.1));
    int bottom = int(round(half_h + 0.1));
    int left = int(round(half_w - 0.1));
    int right = int(round(half_w + 0.1));

    // Add border
    cv::copyMakeBorder(resized_img, resized_img, top, bottom, left, right, 0, cv::Scalar(114, 114, 114));

    return resized_img;
  }

  CommonObjectDetectorIntelImpl::CommonObjectDetectorIntelImpl()
  {
  }

  CommonObjectDetectorIntelImpl::~CommonObjectDetectorIntelImpl()
  {
  }

  void CommonObjectDetectorIntelImpl::intelDetect(
      CommonObjectDetectorBase *base_,
      cv::Mat img_,
      std::vector<float> &boxes_x_,
      std::vector<float> &boxes_y_,
      std::vector<float> &boxes_w_,
      std::vector<float> &boxes_h_,
      std::vector<int> &boxes_label_,
      std::vector<float> &boxes_score_,
      std::vector<cv::Mat> &boxes_seg_,
      bool input_4k_)
  {
#ifdef WITH_INTEL
    int input_h = base_->getInputH();
    int input_w = base_->getInputW();
    bool with_segmentation = base_->withSegmentation();
    double thrs_conf = base_->getThrsConf();
    double thrs_nms = base_->getThrsNms();

    if (with_segmentation)
    {
      std::vector<float> paddings(3); // scale, half_h, half_w
      this->preprocess_img_seg(img_, paddings);

      infer_request.start_async();
      infer_request.wait();

      // Postprocess
      this->postprocess_img_seg(img_, paddings, boxes_x_, boxes_y_, boxes_w_, boxes_h_, boxes_label_, boxes_score_, boxes_seg_, thrs_conf, thrs_nms);
    }
    else
    {
      // Preprocess
      this->preprocess_img(img_);

      // Run inference
      infer_request.start_async();
      infer_request.wait();

      // Postprocess
      this->postprocess_img(boxes_x_, boxes_y_, boxes_w_, boxes_h_, boxes_label_, boxes_score_, thrs_conf, thrs_nms);
    }

#endif
  }

  bool CommonObjectDetectorIntelImpl::intelSetup(CommonObjectDetectorBase *base_, bool input_4k_)
  {
#ifdef WITH_INTEL
    ov::Core core;
    std::string dataset = base_->getDataset();
    double thrs_conf = base_->getThrsConf();
    double thrs_nms = base_->getThrsNms();
    inpHeight = base_->getInputH();
    inpWidth = base_->getInputW();
    with_segmentation = base_->withSegmentation();
    std::string model = base_->getModel();

    std::string openvino_fn = get_home() + SV_MODEL_DIR + dataset + ".onnx";
    std::vector<std::string> files;
    _list_dir(get_home() + SV_MODEL_DIR, files, "-online.onnx", "Int-" + dataset + "-yolov5" + model + "_c");
    if (files.size() > 0)
    {
      std::sort(files.rbegin(), files.rend(), _comp_str_lesser);
      openvino_fn = get_home() + SV_MODEL_DIR + files[0];
    }

    if (inpWidth == 1280)
    {
      files.clear();
      _list_dir(get_home() + SV_MODEL_DIR, files, "-online.onnx", "Int-" + dataset + "-yolov5" + model + "6_c");
      if (files.size() > 0)
      {
        std::sort(files.rbegin(), files.rend(), _comp_str_lesser);
        openvino_fn = get_home() + SV_MODEL_DIR + files[0];
      }
      else
      {
        openvino_fn = get_home() + SV_MODEL_DIR + dataset + "_HD.onnx";
      }
    }
    if (with_segmentation)
    {
      base_->setInputH(640);
      base_->setInputW(640);
      files.clear();
      _list_dir(get_home() + SV_MODEL_DIR, files, "-online.onnx", "Int-" + dataset + "-yolov5" + model + "_seg_c");
      if (files.size() > 0)
      {
        std::sort(files.rbegin(), files.rend(), _comp_str_lesser);
        openvino_fn = get_home() + SV_MODEL_DIR + files[0];
      }
      else
      {
        openvino_fn = get_home() + SV_MODEL_DIR + dataset + "_SEG.onnx";
      }
    }
    std::cout << "Load: " << openvino_fn << std::endl;
    if (!is_file_exist(openvino_fn))
    {
      throw std::runtime_error("SpireCV (104) Error loading the CommonObject OpenVINO model (File Not Exist)");
    }

    if (input_4k_ && with_segmentation)
    {
      throw std::runtime_error("SpireCV (106) Resolution 4K DO NOT Support Segmentation!");
    }

    if (with_segmentation)
    {
      this->compiled_model = core.compile_model(openvino_fn, "GPU");
      this->infer_request = compiled_model.create_infer_request();
    }
    else
    {
      std::shared_ptr<ov::Model> model_ = core.read_model(openvino_fn);
      ov::preprocess::PrePostProcessor Pre_P = ov::preprocess::PrePostProcessor(model_);
      Pre_P.input().tensor().set_element_type(ov::element::u8).set_layout("NHWC").set_color_format(ov::preprocess::ColorFormat::RGB);
      Pre_P.input().preprocess().convert_element_type(ov::element::f32).convert_color(ov::preprocess::ColorFormat::RGB).scale({255, 255, 255}); // .scale({ 112, 112, 112 });
      Pre_P.input().model().set_layout("NCHW");
      Pre_P.output().tensor().set_element_type(ov::element::f32);
      model_ = Pre_P.build();
      this->compiled_model = core.compile_model(model_, "GPU");
      this->infer_request = compiled_model.create_infer_request();
    }
    return true;
#endif
    return false;
  }

  void CommonObjectDetectorIntelImpl::preprocess_img(cv::Mat &img_)
  {
#ifdef WITH_INTEL
    float width = img_.cols;
    float height = img_.rows;
    cv::Size new_shape = cv::Size(inpHeight, inpWidth);
    float r = float(new_shape.width / max(width, height));
    int new_unpadW = int(round(width * r));
    int new_unpadH = int(round(height * r));

    cv::resize(img_, resize.resized_image, cv::Size(new_unpadW, new_unpadH), 0, 0, cv::INTER_AREA);
    resize.resized_image = resize.resized_image;
    resize.dw = new_shape.width - new_unpadW;
    resize.dh = new_shape.height - new_unpadH;
    cv::Scalar color = cv::Scalar(100, 100, 100);
    cv::copyMakeBorder(resize.resized_image, resize.resized_image, 0, resize.dh, 0, resize.dw, cv::BORDER_CONSTANT, color);

    this->rx = (float)img_.cols / (float)(resize.resized_image.cols - resize.dw);
    this->ry = (float)img_.rows / (float)(resize.resized_image.rows - resize.dh);
    if (with_segmentation)
    {
      cv::Mat blob = cv::dnn::blobFromImage(resize.resized_image, 1 / 255.0, cv::Size(640, 640), cv::Scalar(0, 0, 0), true);
      auto input_port = compiled_model.input();
      ov::Tensor input_tensor(input_port.get_element_type(), input_port.get_shape(), blob.ptr(0));
      infer_request.set_input_tensor(input_tensor);
    }
    else
    {
      float *input_data = (float *)resize.resized_image.data;
      input_tensor = ov::Tensor(compiled_model.input().get_element_type(), compiled_model.input().get_shape(), input_data);
      infer_request.set_input_tensor(input_tensor);
    }
#endif
  }

  void CommonObjectDetectorIntelImpl::preprocess_img_seg(cv::Mat &img_, std::vector<float> &paddings)
  {
#ifdef WITH_INTEL
    cv::Mat masked_img;
    cv::Mat resized_img = letterbox(img_, paddings); // resize to (640,640) by letterbox
    // BGR->RGB, u8(0-255)->f32(0.0-1.0), HWC->NCHW
    cv::Mat blob = cv::dnn::blobFromImage(resized_img, 1 / 255.0, cv::Size(640, 640), cv::Scalar(0, 0, 0), true);

    // Get input port for model with one input
    auto input_port = compiled_model.input();
    // Create tensor from external memory
    ov::Tensor input_tensor(input_port.get_element_type(), input_port.get_shape(), blob.ptr(0));
    // Set input tensor for model with one input
    infer_request.set_input_tensor(input_tensor);
#endif
  }

  void CommonObjectDetectorIntelImpl::postprocess_img_seg(cv::Mat &img_,
                                                          std::vector<float> &paddings,
                                                          std::vector<float> &boxes_x_,
                                                          std::vector<float> &boxes_y_,
                                                          std::vector<float> &boxes_w_,
                                                          std::vector<float> &boxes_h_,
                                                          std::vector<int> &boxes_label_,
                                                          std::vector<float> &boxes_score_,
                                                          std::vector<cv::Mat> &boxes_seg_,
                                                          double &thrs_conf,
                                                          double &thrs_nms)
  {
#ifdef WITH_INTEL
    const ov::Tensor &detect = infer_request.get_output_tensor(0);
    ov::Shape detect_shape = detect.get_shape();
    const ov::Tensor &proto = infer_request.get_output_tensor(1);
    ov::Shape proto_shape = proto.get_shape();

    cv::Mat detect_buffer(detect_shape[1], detect_shape[2], CV_32F, detect.data());
    cv::Mat proto_buffer(proto_shape[1], proto_shape[2] * proto_shape[3], CV_32F, proto.data());

    cv::RNG rng;
    float conf_threshold = thrs_conf;
    float nms_threshold = thrs_nms;
    std::vector<cv::Rect> boxes;
    std::vector<int> class_ids;
    std::vector<float> class_scores;
    std::vector<float> confidences;
    std::vector<cv::Mat> masks;

    float scale = paddings[0];
    for (int i = 0; i < detect_buffer.rows; i++)
    {
      float confidence = detect_buffer.at<float>(i, 4);
      if (confidence < conf_threshold)
      {
        continue;
      }
      cv::Mat classes_scores = detect_buffer.row(i).colRange(5, 85);
      cv::Point class_id;
      double score;
      cv::minMaxLoc(classes_scores, NULL, &score, NULL, &class_id);

      // class score: 0~1
      if (score > 0.25)
      {
        cv::Mat mask = detect_buffer.row(i).colRange(85, 117);
        float cx = detect_buffer.at<float>(i, 0);
        float cy = detect_buffer.at<float>(i, 1);
        float w = detect_buffer.at<float>(i, 2);
        float h = detect_buffer.at<float>(i, 3);
        int left = static_cast<int>((cx - 0.5 * w - paddings[2]) / scale);
        int top = static_cast<int>((cy - 0.5 * h - paddings[1]) / scale);
        int width = static_cast<int>(w / scale);
        int height = static_cast<int>(h / scale);
        cv::Rect box;
        box.x = left;
        box.y = top;
        box.width = width;
        box.height = height;

        boxes.push_back(box);
        class_ids.push_back(class_id.x);
        class_scores.push_back(score);
        confidences.push_back(confidence);
        masks.push_back(mask);
      }
    }

    // NMS
    std::vector<int> indices;
    cv::dnn::NMSBoxes(boxes, confidences, thrs_conf, thrs_nms, indices);
    // cv::Mat rgb_mask;
    cv::Mat rgb_mask = cv::Mat::zeros(img_.size(), img_.type());

    for (size_t i = 0; i < indices.size(); i++)
    {
      int index = indices[i];
      int class_id = class_ids[index];
      cv::Rect box = boxes[index];
      int x1 = std::max(0, box.x);
      int y1 = std::max(0, box.y);
      int x2 = std::max(0, box.br().x);
      int y2 = std::max(0, box.br().y);

      cv::Mat m = masks[index] * proto_buffer;
      for (int col = 0; col < m.cols; col++)
      {
        m.at<float>(0, col) = sigmoid_function(m.at<float>(0, col));
      }
      cv::Mat m1 = m.reshape(1, 160); // 1x25600 -> 160x160
      int mx1 = std::max(0, int((x1 * scale + paddings[2]) * 0.25));
      int mx2 = std::max(0, int((x2 * scale + paddings[2]) * 0.25));
      int my1 = std::max(0, int((y1 * scale + paddings[1]) * 0.25));
      int my2 = std::max(0, int((y2 * scale + paddings[1]) * 0.25));
      cv::Mat mask_roi = m1(cv::Range(my1, my2), cv::Range(mx1, mx2));

      cv::Mat rm, det_mask;
      cv::resize(mask_roi, rm, cv::Size(x2 - x1, y2 - y1));
      for (int r = 0; r < rm.rows; r++)
      {
        for (int c = 0; c < rm.cols; c++)
        {
          float pv = rm.at<float>(r, c);
          if (pv > 0.5)
          {
            rm.at<float>(r, c) = 1.0;
          }
          else
          {
            rm.at<float>(r, c) = 0.0;
          }
        }
      }

      rm = rm * rng.uniform(0, 255);
      rm.convertTo(det_mask, CV_8UC1);
      if ((y1 + det_mask.rows) >= img_.rows)
      {
        y2 = img_.rows - 1;
      }
      if ((x1 + det_mask.cols) >= img_.cols)
      {
        x2 = img_.cols - 1;
      }

      cv::Mat mask = cv::Mat::zeros(cv::Size(img_.cols, img_.rows), CV_8UC1);
      det_mask(cv::Range(0, y2 - y1), cv::Range(0, x2 - x1)).copyTo(mask(cv::Range(y1, y2), cv::Range(x1, x2)));
      add(rgb_mask, cv::Scalar(rng.uniform(0, 255), rng.uniform(0, 255), rng.uniform(0, 255)), rgb_mask, mask);

      boxes_x_.push_back(box.x);
      boxes_y_.push_back(box.y);
      boxes_w_.push_back(box.width);
      boxes_h_.push_back(box.height);

      boxes_label_.push_back((int)class_id);
      boxes_score_.push_back(class_scores[index]);

      cv::Mat mask_j = mask.clone();
      boxes_seg_.push_back(mask_j);
    }

#endif
  }

  void CommonObjectDetectorIntelImpl::postprocess_img(std::vector<float> &boxes_x_,
                                                      std::vector<float> &boxes_y_,
                                                      std::vector<float> &boxes_w_,
                                                      std::vector<float> &boxes_h_,
                                                      std::vector<int> &boxes_label_,
                                                      std::vector<float> &boxes_score_,
                                                      double &thrs_conf,
                                                      double &thrs_nms)
  {
#ifdef WITH_INTEL
    const ov::Tensor &output_tensor = infer_request.get_output_tensor();
    ov::Shape output_shape = output_tensor.get_shape();
    float *detections = output_tensor.data<float>();

    std::vector<cv::Rect> boxes;
    vector<int> class_ids;
    vector<float> confidences;
    for (int i = 0; i < output_shape[1]; i++)
    {
      float *detection = &detections[i * output_shape[2]];

      float confidence = detection[4];
      if (confidence >= thrs_conf)
      {
        float *classes_scores = &detection[5];
        cv::Mat scores(1, output_shape[2] - 5, CV_32FC1, classes_scores);
        cv::Point class_id;
        double max_class_score;
        cv::minMaxLoc(scores, 0, &max_class_score, 0, &class_id);
        if (max_class_score > thrs_conf)
        {
          confidences.push_back(confidence);
          class_ids.push_back(class_id.x);
          float x = detection[0];
          float y = detection[1];
          float w = detection[2];
          float h = detection[3];
          float xmin = x - (w / 2);
          float ymin = y - (h / 2);

          boxes.push_back(cv::Rect(xmin, ymin, w, h));
        }
      }
    }

    std::vector<int> nms_result;
    cv::dnn::NMSBoxes(boxes, confidences, thrs_conf, thrs_nms, nms_result);

    std::vector<Detection> output;
    for (int i = 0; i < nms_result.size(); i++)
    {
      Detection result;
      int idx = nms_result[i];
      result.class_id = class_ids[idx];
      result.confidence = confidences[idx];
      result.box = boxes[idx];
      output.push_back(result);
    }

    for (int i = 0; i < output.size(); i++)
    {
      auto detection = output[i];
      auto box = detection.box;
      auto classId = detection.class_id;
      auto confidence = detection.confidence;

      float xmax = box.x + box.width;
      float ymax = box.y + box.height;

      boxes_x_.push_back(this->rx * box.x);
      boxes_y_.push_back(this->ry * box.y);
      boxes_w_.push_back(this->rx * box.width);
      boxes_h_.push_back(this->ry * box.height);

      boxes_label_.push_back((int)detection.class_id);
      boxes_score_.push_back(detection.confidence);
    }
#endif
  }

}
