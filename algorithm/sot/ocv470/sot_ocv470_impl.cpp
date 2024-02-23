#include "sot_ocv470_impl.h"
#include <cmath>
#include <fstream>
#include "sv_util.h"

#define SV_MODEL_DIR "/SpireCV/models/"
#define SV_ROOT_DIR "/SpireCV/"


namespace sv {

using namespace cv;


SingleObjectTrackerOCV470Impl::SingleObjectTrackerOCV470Impl()
{
}

SingleObjectTrackerOCV470Impl::~SingleObjectTrackerOCV470Impl()
{
}


bool SingleObjectTrackerOCV470Impl::ocv470Setup(SingleObjectTrackerBase* base_)
{
  this->_algorithm = base_->getAlgorithm();
  this->_backend = base_->getBackend();
  this->_target = base_->getTarget();

#ifdef WITH_OCV470
  std::string net = get_home() + SV_MODEL_DIR + "dasiamrpn_model.onnx";
  std::string kernel_cls1 = get_home() + SV_MODEL_DIR + "dasiamrpn_kernel_cls1.onnx";
  std::string kernel_r1 = get_home() + SV_MODEL_DIR + "dasiamrpn_kernel_r1.onnx";
  
  std::vector<std::string> files1, files2, files3;
  _list_dir(get_home() + SV_MODEL_DIR, files1, ".onnx", "Ocv-DaSiamRPN-Model-");
  _list_dir(get_home() + SV_MODEL_DIR, files2, ".onnx", "Ocv-DaSiamRPN-Kernel-CLS1-");
  _list_dir(get_home() + SV_MODEL_DIR, files3, ".onnx", "Ocv-DaSiamRPN-Kernel-R1-");
  if (files1.size() > 0 && files2.size() > 0 && files3.size() > 0)
  {
    std::sort(files1.rbegin(), files1.rend(), _comp_str_lesser);
    std::sort(files2.rbegin(), files2.rend(), _comp_str_lesser);
    std::sort(files3.rbegin(), files3.rend(), _comp_str_lesser);
    net = get_home() + SV_MODEL_DIR + files1[0];
    kernel_cls1 = get_home() + SV_MODEL_DIR + files2[0];
    kernel_r1 = get_home() + SV_MODEL_DIR + files3[0];
  }
  std::cout << "Load: " << net << std::endl;
  std::cout << "Load: " << kernel_cls1 << std::endl;
  std::cout << "Load: " << kernel_r1 << std::endl;

  std::string backbone = get_home() + SV_MODEL_DIR + "nanotrack_backbone_sim.onnx";
  std::string neckhead = get_home() + SV_MODEL_DIR + "nanotrack_head_sim.onnx";
  
  std::vector<std::string> files4, files5;
  _list_dir(get_home() + SV_MODEL_DIR, files4, ".onnx", "Ocv-NanoTrack-Backbone-SIM-");
  _list_dir(get_home() + SV_MODEL_DIR, files5, ".onnx", "Ocv-NanoTrack-Head-SIM-");
  if (files4.size() > 0 && files5.size() > 0)
  {
    std::sort(files4.rbegin(), files4.rend(), _comp_str_lesser);
    std::sort(files5.rbegin(), files5.rend(), _comp_str_lesser);
    backbone = get_home() + SV_MODEL_DIR + files4[0];
    neckhead = get_home() + SV_MODEL_DIR + files5[0];
  }
  std::cout << "Load: " << backbone << std::endl;
  std::cout << "Load: " << neckhead << std::endl;

  try
  {
    TrackerNano::Params nano_params;
    nano_params.backbone = samples::findFile(backbone);
    nano_params.neckhead = samples::findFile(neckhead);
    nano_params.backend = this->_backend;
    nano_params.target = this->_target;

    _nano = TrackerNano::create(nano_params);
  }
  catch (const cv::Exception& ee)
  {
    std::cerr << "Exception: " << ee.what() << std::endl;
    std::cout << "Can't load the network by using the following files:" << std::endl;
    std::cout << "nanoBackbone : " << backbone << std::endl;
    std::cout << "nanoNeckhead : " << neckhead << std::endl;
  }

  try
  {
    TrackerDaSiamRPN::Params params;
    params.model = samples::findFile(net);
    params.kernel_cls1 = samples::findFile(kernel_cls1);
    params.kernel_r1 = samples::findFile(kernel_r1);
    params.backend = this->_backend;
    params.target = this->_target;
    _siam_rpn = TrackerDaSiamRPN::create(params);
  }
  catch (const cv::Exception& ee)
  {
    std::cerr << "Exception: " << ee.what() << std::endl;
    std::cout << "Can't load the network by using the following files:" << std::endl;
    std::cout << "siamRPN : " << net << std::endl;
    std::cout << "siamKernelCL1 : " << kernel_cls1 << std::endl;
    std::cout << "siamKernelR1 : " << kernel_r1 << std::endl;
  }
  return true;
#endif
  return false;
}


void SingleObjectTrackerOCV470Impl::ocv470Init(cv::Mat img_, const cv::Rect& bounding_box_)
{
#ifdef WITH_OCV470
  if (this->_algorithm == "kcf")
  {
    TrackerKCF::Params params;
    _kcf = TrackerKCF::create(params);
    _kcf->init(img_, bounding_box_);
  }
  else if (this->_algorithm == "csrt")
  {
    TrackerCSRT::Params params;
    _csrt = TrackerCSRT::create(params);
    _csrt->init(img_, bounding_box_);
  }
  else if (this->_algorithm == "siamrpn")
  {
    _siam_rpn->init(img_, bounding_box_);
  }
  else if (this->_algorithm == "nano")
  {
    _nano->init(img_, bounding_box_);
  }
#endif
}


bool SingleObjectTrackerOCV470Impl::ocv470Track(cv::Mat img_, cv::Rect& output_bbox_)
{
#ifdef WITH_OCV470
  bool ok = false;
  if (this->_algorithm == "kcf")
  {
    ok = _kcf->update(img_, output_bbox_);
  }
  else if (this->_algorithm == "csrt")
  {
    ok = _csrt->update(img_, output_bbox_);
  }
  else if (this->_algorithm == "siamrpn")
  {
    ok = _siam_rpn->update(img_, output_bbox_);
  }
  else if (this->_algorithm == "nano")
  {
    ok = _nano->update(img_, output_bbox_);
  }
  return ok;
#endif
  return false;
}




}

