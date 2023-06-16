#ifndef __SV_ALGORITHM__
#define __SV_ALGORITHM__

#include "sv_video_base.h"
#include <opencv2/opencv.hpp>
#include <opencv2/aruco.hpp>
#include <opencv2/tracking.hpp>
#include <string>
#include <chrono>


namespace yaed {
class EllipseDetector;
}

namespace sv {

// union JsonValue;
// class JsonAllocator;

class CameraAlgorithm
{
public:
  CameraAlgorithm();
  ~CameraAlgorithm();
  void loadCameraParams(std::string yaml_fn_);
  void loadAlgorithmParams(std::string json_fn_);
  
  cv::Mat camera_matrix;
  cv::Mat distortion;
  int image_width;
  int image_height;
  double fov_x;
  double fov_y;
  
  std::string alg_params_fn;
protected:
  // JsonValue* _value;
  // JsonAllocator* _allocator;
  std::chrono::system_clock::time_point _t0;
};


class ArucoDetector : public CameraAlgorithm
{
public:
  ArucoDetector();
  void detect(cv::Mat img_, TargetsInFrame& tgts_);
private:
  void _load();
  bool _params_loaded;
  cv::Ptr<cv::aruco::DetectorParameters> _detector_params;
  cv::Ptr<cv::aruco::Dictionary> _dictionary;
  int _dictionary_id;
  std::vector<int> _ids_need;
  std::vector<double> _lengths_need;
};


class EllipseDetector : public CameraAlgorithm
{
public:
  EllipseDetector();
  ~EllipseDetector();
  void detectAllInDirectory(std::string input_img_dir_, std::string output_json_dir_);
  void detect(cv::Mat img_, TargetsInFrame& tgts_);
protected:
  void _load();
  bool _params_loaded;
  yaed::EllipseDetector* _ed;
  float _max_center_distance_ratio;
  double _radius_in_meter;
};

class LandingMarkerDetectorBase : public EllipseDetector
{
public:
  LandingMarkerDetectorBase();
  ~LandingMarkerDetectorBase();
  void detect(cv::Mat img_, TargetsInFrame& tgts_);
  
  bool isParamsLoaded();
  int getMaxCandidates();
  std::vector<std::string> getLabelsNeed();
protected:
  virtual bool setupImpl();
  virtual void roiCNN(std::vector<cv::Mat>& input_rois_, std::vector<int>& output_labels_);
  void _loadLabels();
  int _max_candidates;
  std::vector<std::string> _labels_need;
};


class SingleObjectTrackerBase : public CameraAlgorithm
{
public:
  SingleObjectTrackerBase();
  ~SingleObjectTrackerBase();
  void warmUp();
  void init(cv::Mat img_, const cv::Rect& bounding_box_);
  void track(cv::Mat img_, TargetsInFrame& tgts_);
  
  bool isParamsLoaded();
  std::string getAlgorithm();
  int getBackend();
  int getTarget();
protected:
  virtual bool setupImpl();
  virtual void initImpl(cv::Mat img_, const cv::Rect& bounding_box_);
  virtual bool trackImpl(cv::Mat img_, cv::Rect& output_bbox_);
  void _load();
  bool _params_loaded;
  std::string _algorithm;
  int _backend;
  int _target;
};


class CommonObjectDetectorBase : public CameraAlgorithm
{
public:
  CommonObjectDetectorBase();
  ~CommonObjectDetectorBase();
  void warmUp();
  void detect(cv::Mat img_, TargetsInFrame& tgts_, Box* roi_=nullptr, int img_w_=0, int img_h_=0);
  
  bool isParamsLoaded();
  std::string getDataset();
  std::vector<std::string> getClassNames();
  std::vector<double> getClassWs();
  std::vector<double> getClassHs();
  int getInputH();
  void setInputH(int h_);
  int getInputW();
  void setInputW(int w_);
  int getClassNum();
  int getOutputSize();
  double getThrsNms();
  double getThrsConf();
  int useWidthOrHeight();
  bool withSegmentation();
protected:
  virtual bool setupImpl();
  virtual void detectImpl(
    cv::Mat img_,
    std::vector<float>& boxes_x_,
    std::vector<float>& boxes_y_,
    std::vector<float>& boxes_w_,
    std::vector<float>& boxes_h_,
    std::vector<int>& boxes_label_,
    std::vector<float>& boxes_score_,
    std::vector<cv::Mat>& boxes_seg_
  );
  void _load();
  bool _params_loaded;
  std::string _dataset;
  std::vector<std::string> _class_names;
  std::vector<double> _class_ws;
  std::vector<double> _class_hs;
  int _input_h;
  int _input_w;
  int _n_classes;
  int _output_size;
  double _thrs_nms;
  double _thrs_conf;
  int _use_width_or_height;
  bool _with_segmentation;
};


}
#endif
