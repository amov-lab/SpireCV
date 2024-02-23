#include "landing_det_intel_impl.h"
#include <cmath>
#include <fstream>

#define SV_MODEL_DIR "/SpireCV/models/"
#define SV_ROOT_DIR "/SpireCV/"

namespace sv
{

#ifdef WITH_INTEL
  using namespace cv;
  using namespace std;
  using namespace dnn;
#endif

  LandingMarkerDetectorIntelImpl::LandingMarkerDetectorIntelImpl()
  {
  }

  LandingMarkerDetectorIntelImpl::~LandingMarkerDetectorIntelImpl()
  {
  }

  bool LandingMarkerDetectorIntelImpl::intelSetup()
  {
#ifdef WITH_INTEL
    std::string onnx_model_fn = get_home() + SV_MODEL_DIR + "LandingMarker.onnx";
    std::vector<std::string> files;
    _list_dir(get_home() + SV_MODEL_DIR, files, "-online.onnx", "Int-LandingMarker-resnet34");
    if (files.size() > 0)
    {
      std::sort(files.rbegin(), files.rend(), _comp_str_lesser);
      onnx_model_fn = get_home() + SV_MODEL_DIR + files[0];
    }

    if (!is_file_exist(onnx_model_fn))
    {
      throw std::runtime_error("SpireCV (104) Error loading the LandingMarker ONNX model (File Not Exist)");
    }

    // OpenVINO
    ov::Core core;
    std::shared_ptr<ov::Model> model = core.read_model(onnx_model_fn);
    ov::preprocess::PrePostProcessor ppp = ov::preprocess::PrePostProcessor(model);
    ppp.input().tensor().set_element_type(ov::element::u8).set_layout("NHWC").set_color_format(ov::preprocess::ColorFormat::RGB);
    ppp.input().preprocess().convert_element_type(ov::element::f32).convert_color(ov::preprocess::ColorFormat::RGB).scale({255, 255, 255}); // .scale({ 112, 112, 112 });
    ppp.input().model().set_layout("NCHW");
    ppp.output().tensor().set_element_type(ov::element::f32);
    model = ppp.build();
    this->compiled_model = core.compile_model(model, "GPU");
    this->infer_request = compiled_model.create_infer_request();

    return true;
#endif
    return false;
  }

  void LandingMarkerDetectorIntelImpl::intelRoiCNN(
      std::vector<cv::Mat> &input_rois_,
      std::vector<int> &output_labels_)
  {
#ifdef WITH_INTEL
    output_labels_.clear();

    for (int i = 0; i < input_rois_.size(); i++)
    {
      cv::Mat e_roi = input_rois_[i];

      // Get input port for model with one input
      auto input_port = compiled_model.input();
      // Create tensor from external memory
      ov::Tensor input_tensor(input_port.get_element_type(), input_port.get_shape(), e_roi.ptr(0));
      // Set input tensor for model with one input
      infer_request.set_input_tensor(input_tensor);
      //preprocess_img(e_roi);

      // infer_request.infer();
      infer_request.start_async();
      infer_request.wait();

      const ov::Tensor &output_tensor = infer_request.get_output_tensor();
      ov::Shape output_shape = output_tensor.get_shape();
      this->_p_prob = output_tensor.data<float>();

      // Find max index
      double max = 0;
      int label = 0;
      for (int i = 0; i < 11; ++i)
      {
        if (max < this->_p_prob[i])
        {
          max = this->_p_prob[i];
          label = i;
        }
      }
      output_labels_.push_back(label);
    }

#endif
  }

}
