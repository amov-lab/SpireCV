#include "sv_algorithm_base.h"
#include <cmath>
#include <opencv2/objdetect/aruco_detector.hpp>
#include <opencv2/calib3d.hpp>
#include <opencv2/dnn.hpp>
#include "gason.h"
#include <fstream>
#include "sv_util.h"
#include "ellipse_detector.h"

#define SV_MODEL_DIR "/SpireCV/models/"
#define SV_ROOT_DIR "/SpireCV/"


namespace sv {

using namespace cv;
using namespace cv::dnn;


void _cameraMatrix2Fov(cv::Mat camera_matrix_, int img_w_, int img_h_, double& fov_x_, double& fov_y_)
{
  fov_x_ = 2 * atan(img_w_ / 2. / camera_matrix_.at<double>(0, 0)) * SV_RAD2DEG;
  fov_y_ = 2 * atan(img_h_ / 2. / camera_matrix_.at<double>(1, 1)) * SV_RAD2DEG;
}



CameraAlgorithm::CameraAlgorithm()
{
  // this->_value = NULL;
  // this->_allocator = NULL;
  this->_t0 = std::chrono::system_clock::now();

  this->alg_params_fn = _get_home() + SV_ROOT_DIR + "sv_algorithm_params.json";
  // std::cout << "CameraAlgorithm->alg_params_fn: " << this->alg_params_fn << std::endl;
  // if (_is_file_exist(params_fn))
  //   this->loadAlgorithmParams(params_fn);
}
CameraAlgorithm::~CameraAlgorithm()
{
  // if (_value) delete _value;
  // if (_allocator) delete _allocator;
}

void CameraAlgorithm::loadCameraParams(std::string yaml_fn_)
{
  cv::FileStorage fs(yaml_fn_, cv::FileStorage::READ);
  if (!fs.isOpened())
  {
    throw std::runtime_error("SpireCV (104) Camera calib file NOT exist!");
  }
  fs["camera_matrix"] >> this->camera_matrix;
  fs["distortion_coefficients"] >> this->distortion;
  fs["image_width"] >> this->image_width;
  fs["image_height"] >> this->image_height;
  
  if (this->camera_matrix.rows != 3 || this->camera_matrix.cols != 3 ||
      this->distortion.rows != 1 || this->distortion.cols != 5 ||
      this->image_width == 0 || this->image_height == 0)
  {
    throw std::runtime_error("SpireCV (106) Camera parameters reading ERROR!");
  }
  
  _cameraMatrix2Fov(this->camera_matrix, this->image_width, this->image_height, this->fov_x, this->fov_y);
  // std::cout << this->fov_x << ", " << this->fov_y << std::endl;
}


void CameraAlgorithm::loadAlgorithmParams(std::string json_fn_)
{
  this->alg_params_fn = json_fn_;
}


ArucoDetector::ArucoDetector()
{
  _params_loaded = false;
  _dictionary = nullptr;
}


void ArucoDetector::_load()
{
  JsonValue all_value;
  JsonAllocator allocator;
  std::cout << "Load: [" << this->alg_params_fn << "]" << std::endl;
  _load_all_json(this->alg_params_fn, all_value, allocator);

  JsonValue aruco_params_value;
  _parser_algorithm_params("ArucoDetector", all_value, aruco_params_value);
  
  _dictionary_id = 10;
  // _detector_params = aruco::DetectorParameters::create();
  _detector_params = new aruco::DetectorParameters;
  for (auto i : aruco_params_value) {
    if ("_dictionary_id" == std::string(i->key)) {
      _dictionary_id = i->value.toNumber();
    }
    else if ("adaptiveThreshConstant" == std::string(i->key)) {
      // std::cout << "adaptiveThreshConstant (old, new): " << _detector_params->adaptiveThreshConstant << ", " << i->value.toNumber() << std::endl;
      _detector_params->adaptiveThreshConstant = i->value.toNumber();
    }
    else if ("adaptiveThreshWinSizeMax" == std::string(i->key)) {
      // std::cout << "adaptiveThreshWinSizeMax (old, new): " << _detector_params->adaptiveThreshWinSizeMax << ", " << i->value.toNumber() << std::endl;
      _detector_params->adaptiveThreshWinSizeMax = i->value.toNumber();
    }
    else if ("adaptiveThreshWinSizeMin" == std::string(i->key)) {
      // std::cout << "adaptiveThreshWinSizeMin (old, new): " << _detector_params->adaptiveThreshWinSizeMin << ", " << i->value.toNumber() << std::endl;
      _detector_params->adaptiveThreshWinSizeMin = i->value.toNumber();
    }
    else if ("adaptiveThreshWinSizeStep" == std::string(i->key)) {
      // std::cout << "adaptiveThreshWinSizeStep (old, new): " << _detector_params->adaptiveThreshWinSizeStep << ", " << i->value.toNumber() << std::endl;
      _detector_params->adaptiveThreshWinSizeStep = i->value.toNumber();
    }
    else if ("aprilTagCriticalRad" == std::string(i->key)) {
      // std::cout << "aprilTagCriticalRad (old, new): " << _detector_params->aprilTagCriticalRad << ", " << i->value.toNumber() << std::endl;
      _detector_params->aprilTagCriticalRad = i->value.toNumber();
    }
    else if ("aprilTagDeglitch" == std::string(i->key)) {
      // std::cout << "aprilTagDeglitch (old, new): " << _detector_params->aprilTagDeglitch << ", " << i->value.toNumber() << std::endl;
      _detector_params->aprilTagDeglitch = i->value.toNumber();
    }
    else if ("aprilTagMaxLineFitMse" == std::string(i->key)) {
      // std::cout << "aprilTagMaxLineFitMse (old, new): " << _detector_params->aprilTagMaxLineFitMse << ", " << i->value.toNumber() << std::endl;
      _detector_params->aprilTagMaxLineFitMse = i->value.toNumber();
    }
    else if ("aprilTagMaxNmaxima" == std::string(i->key)) {
      // std::cout << "aprilTagMaxNmaxima (old, new): " << _detector_params->aprilTagMaxNmaxima << ", " << i->value.toNumber() << std::endl;
      _detector_params->aprilTagMaxNmaxima = i->value.toNumber();
    }
    else if ("aprilTagMinClusterPixels" == std::string(i->key)) {
      // std::cout << "aprilTagMinClusterPixels (old, new): " << _detector_params->aprilTagMinClusterPixels << ", " << i->value.toNumber() << std::endl;
      _detector_params->aprilTagMinClusterPixels = i->value.toNumber();
    }
    else if ("aprilTagMinWhiteBlackDiff" == std::string(i->key)) {
      // std::cout << "aprilTagMinWhiteBlackDiff (old, new): " << _detector_params->aprilTagMinWhiteBlackDiff << ", " << i->value.toNumber() << std::endl;
      _detector_params->aprilTagMinWhiteBlackDiff = i->value.toNumber();
    }
    else if ("aprilTagQuadDecimate" == std::string(i->key)) {
      // std::cout << "aprilTagQuadDecimate (old, new): " << _detector_params->aprilTagQuadDecimate << ", " << i->value.toNumber() << std::endl;
      _detector_params->aprilTagQuadDecimate = i->value.toNumber();
    }
    else if ("aprilTagQuadSigma" == std::string(i->key)) {
      // std::cout << "aprilTagQuadSigma (old, new): " << _detector_params->aprilTagQuadSigma << ", " << i->value.toNumber() << std::endl;
      _detector_params->aprilTagQuadSigma = i->value.toNumber();
    }
    else if ("cornerRefinementMaxIterations" == std::string(i->key)) {
      // std::cout << "cornerRefinementMaxIterations (old, new): " << _detector_params->cornerRefinementMaxIterations << ", " << i->value.toNumber() << std::endl;
      _detector_params->cornerRefinementMaxIterations = i->value.toNumber();
    }
    else if ("cornerRefinementMethod" == std::string(i->key)) {
      // std::cout << "cornerRefinementMethod (old, new): " << _detector_params->cornerRefinementMethod << ", " << i->value.toNumber() << std::endl;
      // _detector_params->cornerRefinementMethod = i->value.toNumber();
      int method = (int) i->value.toNumber();
      if (method == 1)
      {
        _detector_params->cornerRefinementMethod = cv::aruco::CornerRefineMethod::CORNER_REFINE_SUBPIX;
      }
      else if (method == 2)
      {
        _detector_params->cornerRefinementMethod = cv::aruco::CornerRefineMethod::CORNER_REFINE_CONTOUR;
      }
      else if (method == 3)
      {
        _detector_params->cornerRefinementMethod = cv::aruco::CornerRefineMethod::CORNER_REFINE_APRILTAG;
      }
      else
      {
        _detector_params->cornerRefinementMethod = cv::aruco::CornerRefineMethod::CORNER_REFINE_NONE;
      }
    }
    else if ("cornerRefinementMinAccuracy" == std::string(i->key)) {
      // std::cout << "cornerRefinementMinAccuracy (old, new): " << _detector_params->cornerRefinementMinAccuracy << ", " << i->value.toNumber() << std::endl;
      _detector_params->cornerRefinementMinAccuracy = i->value.toNumber();
    }
    else if ("cornerRefinementWinSize" == std::string(i->key)) {
      // std::cout << "cornerRefinementWinSize (old, new): " << _detector_params->cornerRefinementWinSize << ", " << i->value.toNumber() << std::endl;
      _detector_params->cornerRefinementWinSize = i->value.toNumber();
    }
    else if ("detectInvertedMarker" == std::string(i->key)) {
      bool json_tf = false;
      if (i->value.getTag() == JSON_TRUE)  json_tf = true;
      // std::cout << "detectInvertedMarker (old, new): " << _detector_params->detectInvertedMarker << ", " << json_tf << std::endl;
      _detector_params->detectInvertedMarker = json_tf;
    }
    else if ("errorCorrectionRate" == std::string(i->key)) {
      // std::cout << "errorCorrectionRate (old, new): " << _detector_params->errorCorrectionRate << ", " << i->value.toNumber() << std::endl;
      _detector_params->errorCorrectionRate = i->value.toNumber();
    }
    else if ("markerBorderBits" == std::string(i->key)) {
      // std::cout << "markerBorderBits (old, new): " << _detector_params->markerBorderBits << ", " << i->value.toNumber() << std::endl;
      _detector_params->markerBorderBits = i->value.toNumber();
    }
    else if ("maxErroneousBitsInBorderRate" == std::string(i->key)) {
      // std::cout << "maxErroneousBitsInBorderRate (old, new): " << _detector_params->maxErroneousBitsInBorderRate << ", " << i->value.toNumber() << std::endl;
      _detector_params->maxErroneousBitsInBorderRate = i->value.toNumber();
    }
    else if ("maxMarkerPerimeterRate" == std::string(i->key)) {
      // std::cout << "maxMarkerPerimeterRate (old, new): " << _detector_params->maxMarkerPerimeterRate << ", " << i->value.toNumber() << std::endl;
      _detector_params->maxMarkerPerimeterRate = i->value.toNumber();
    }
    else if ("minCornerDistanceRate" == std::string(i->key)) {
      // std::cout << "minCornerDistanceRate (old, new): " << _detector_params->minCornerDistanceRate << ", " << i->value.toNumber() << std::endl;
      _detector_params->minCornerDistanceRate = i->value.toNumber();
    }
    else if ("minDistanceToBorder" == std::string(i->key)) {
      // std::cout << "minDistanceToBorder (old, new): " << _detector_params->minDistanceToBorder << ", " << i->value.toNumber() << std::endl;
      _detector_params->minDistanceToBorder = i->value.toNumber();
    }
    else if ("minMarkerDistanceRate" == std::string(i->key)) {
      // std::cout << "minMarkerDistanceRate (old, new): " << _detector_params->minMarkerDistanceRate << ", " << i->value.toNumber() << std::endl;
      _detector_params->minMarkerDistanceRate = i->value.toNumber();
    }
    else if ("minMarkerLengthRatioOriginalImg" == std::string(i->key)) {
      // std::cout << "minMarkerLengthRatioOriginalImg (old, new): " << _detector_params->minMarkerLengthRatioOriginalImg << ", " << i->value.toNumber() << std::endl;
      _detector_params->minMarkerLengthRatioOriginalImg = i->value.toNumber();
    }
    else if ("minMarkerPerimeterRate" == std::string(i->key)) {
      // std::cout << "minMarkerPerimeterRate (old, new): " << _detector_params->minMarkerPerimeterRate << ", " << i->value.toNumber() << std::endl;
      _detector_params->minMarkerPerimeterRate = i->value.toNumber();
    }
    else if ("minOtsuStdDev" == std::string(i->key)) {
      // std::cout << "minOtsuStdDev (old, new): " << _detector_params->minOtsuStdDev << ", " << i->value.toNumber() << std::endl;
      _detector_params->minOtsuStdDev = i->value.toNumber();
    }
    else if ("minSideLengthCanonicalImg" == std::string(i->key)) {
      // std::cout << "minSideLengthCanonicalImg (old, new): " << _detector_params->minSideLengthCanonicalImg << ", " << i->value.toNumber() << std::endl;
      _detector_params->minSideLengthCanonicalImg = i->value.toNumber();
    }
    else if ("perspectiveRemoveIgnoredMarginPerCell" == std::string(i->key)) {
      // std::cout << "perspectiveRemoveIgnoredMarginPerCell (old, new): " << _detector_params->perspectiveRemoveIgnoredMarginPerCell << ", " << i->value.toNumber() << std::endl;
      _detector_params->perspectiveRemoveIgnoredMarginPerCell = i->value.toNumber();
    }
    else if ("perspectiveRemovePixelPerCell" == std::string(i->key)) {
      // std::cout << "perspectiveRemovePixelPerCell (old, new): " << _detector_params->perspectiveRemovePixelPerCell << ", " << i->value.toNumber() << std::endl;
      _detector_params->perspectiveRemovePixelPerCell = i->value.toNumber();
    }
    else if ("polygonalApproxAccuracyRate" == std::string(i->key)) {
      // std::cout << "polygonalApproxAccuracyRate (old, new): " << _detector_params->polygonalApproxAccuracyRate << ", " << i->value.toNumber() << std::endl;
      _detector_params->polygonalApproxAccuracyRate = i->value.toNumber();
    }
    else if ("useAruco3Detection" == std::string(i->key)) {
      bool json_tf = false;
      if (i->value.getTag() == JSON_TRUE)  json_tf = true;
      // std::cout << "useAruco3Detection (old, new): " << _detector_params->useAruco3Detection << ", " << json_tf << std::endl;
      _detector_params->useAruco3Detection = json_tf;
    }
    else if ("markerIds" == std::string(i->key) && i->value.getTag() == JSON_ARRAY) {
      int jcnt = 0;
      for (auto j : i->value) {
        if (jcnt == 0 && j->value.toNumber() == -1) {
          _ids_need.push_back(-1);
          break;
        }
        else {
          _ids_need.push_back(j->value.toNumber());
        }
      }
    }
    else if ("markerLengths" == std::string(i->key) && i->value.getTag() == JSON_ARRAY) {
      for (auto j : i->value) {
        if (_ids_need.size() > 0 && _ids_need[0] == -1) {
          _lengths_need.push_back(j->value.toNumber());
          break;
        }
        else {
          _lengths_need.push_back(j->value.toNumber());
        }
      }
    }
  }

  if (_ids_need.size() == 0)  _ids_need.push_back(-1);
  if (_lengths_need.size() != _ids_need.size()) {
    throw std::runtime_error("SpireCV (106) Parameter markerIds.length != markerLengths.length!");
  }

  // for (int id : _ids_need)
  //   std::cout << "_ids_need: " << id << std::endl;
  // for (double l : _lengths_need)
  //   std::cout << "_lengths_need: " << l << std::endl;
}


void ArucoDetector::detect(cv::Mat img_, TargetsInFrame& tgts_)
{
  if (!_params_loaded)
  {
    this->_load();
    _params_loaded = true;
  }
  if (img_.cols != this->image_width || img_.rows != this->image_height)
  {
    char msg[256];
    sprintf(msg, "SpireCV (106) Calib camera SIZE(%d) != Input image SIZE(%d)!", this->image_width, img_.cols);
    throw std::runtime_error(msg);
  }
  // std::cout << "_dictionary_id: " << _dictionary_id << std::endl;
  // Ptr<aruco::Dictionary> dictionary = aruco::getPredefinedDictionary(aruco::PredefinedDictionaryType(_dictionary_id));
  if (this->_dictionary == nullptr)
  {
    this->_dictionary = new aruco::Dictionary;
    *(this->_dictionary) = aruco::getPredefinedDictionary(aruco::PredefinedDictionaryType(_dictionary_id));
  }

  std::vector<int> ids, ids_final;
  std::vector<std::vector<cv::Point2f> > corners, corners_final, rejected;
  std::vector<cv::Vec3d> rvecs, tvecs;

  // detect markers and estimate pose
  aruco::detectMarkers(img_, this->_dictionary, corners, ids, _detector_params, rejected);


  if (ids.size() > 0)
  {
    if (_ids_need[0] == -1)
    {
      // std::cout << this->camera_matrix << std::endl;
      aruco::estimatePoseSingleMarkers(corners, _lengths_need[0], this->camera_matrix, this->distortion, rvecs, tvecs);
      ids_final = ids;
      corners_final = corners;
    }
    else 
    {
      for (int i=0; i<_ids_need.size(); i++)
      {
        int id_need = _ids_need[i];
        double length_need = _lengths_need[i];
        std::vector<cv::Vec3d> t_rvecs, t_tvecs;
        std::vector<std::vector<cv::Point2f> > t_corners;
        for (int j=0; j<ids.size(); j++)
        {
          if (ids[j] == id_need)
          {
            t_corners.push_back(corners[j]);
            ids_final.push_back(ids[j]);
            corners_final.push_back(corners[j]);
          }
        }
        if (t_corners.size() > 0)
        {
          aruco::estimatePoseSingleMarkers(t_corners, length_need, this->camera_matrix, this->distortion, t_rvecs, t_tvecs);
          for (auto t_rvec : t_rvecs)
            rvecs.push_back(t_rvec);
          for (auto t_tvec : t_tvecs)
            tvecs.push_back(t_tvec);
        }
      }
    }
  }

  // aruco::drawDetectedMarkers(img_, corners_final, ids_final);
  tgts_.setSize(img_.cols, img_.rows);

  // tgts_.fov_x = this->fov_x;
  // tgts_.fov_y = this->fov_y;
  tgts_.setFOV(this->fov_x, this->fov_y);
  auto t1 = std::chrono::system_clock::now();
  tgts_.setFPS(1000.0 / std::chrono::duration_cast<std::chrono::milliseconds>(t1 - this->_t0).count());
  this->_t0 = std::chrono::system_clock::now();
  tgts_.setTimeNow();

  if (ids_final.size() > 0)
  {
    for (int i=0; i<ids_final.size(); i++)
    {
      Target tgt;
      tgt.setAruco(ids_final[i], corners_final[i], rvecs[i], tvecs[i], tgts_.width, tgts_.height, this->camera_matrix);
      tgts_.targets.push_back(tgt);

      // Box b;
      // tgt.getBox(b);
      // cv::circle(img_, cv::Point(int(corners_final[i][0].x), int(corners_final[i][0].y)), 5, cv::Scalar(0,0,255), 2);
      // cv::circle(img_, cv::Point(int(corners_final[i][1].x), int(corners_final[i][1].y)), 5, cv::Scalar(255,0,0), 2);
      // cv::rectangle(img_, cv::Rect(b.x1, b.y1, b.x2-b.x1+1, b.y2-b.y1+1), cv::Scalar(0,0,255), 1, 1, 0);
    }
  }

  tgts_.type = MissionType::ARUCO_DET;

  // imshow("img", img_);
  // waitKey(10);
}



EllipseDetector::EllipseDetector()
{
  this->_ed = NULL;
  this->_max_center_distance_ratio = 0.05f;
  this->_params_loaded = false;
}
EllipseDetector::~EllipseDetector()
{
  if (_ed) { delete _ed; _ed = NULL; }
}

void EllipseDetector::detect(cv::Mat img_, TargetsInFrame& tgts_)
{
  if (!_params_loaded)
  {
    this->_load();
    _params_loaded = true;
  }
  
  float fMaxCenterDistance = sqrt(float(img_.cols*img_.cols + img_.rows*img_.rows)) * this->_max_center_distance_ratio;
  _ed->SetMCD(fMaxCenterDistance);
  std::vector<yaed::Ellipse> ellsCned;
  _ed->Detect(img_, ellsCned);
  
  tgts_.setSize(img_.cols, img_.rows);
  tgts_.setFOV(this->fov_x, this->fov_y);
  auto t1 = std::chrono::system_clock::now();
  tgts_.setFPS(1000.0 / std::chrono::duration_cast<std::chrono::milliseconds>(t1 - this->_t0).count());
  this->_t0 = std::chrono::system_clock::now();
  tgts_.setTimeNow();
  
  for (yaed::Ellipse ell : ellsCned)
  {
    Target tgt;
    tgt.setEllipse(ell.xc_, ell.yc_, ell.a_, ell.b_, ell.rad_, ell.score_, tgts_.width, tgts_.height, this->camera_matrix, this->_radius_in_meter);
    tgts_.targets.push_back(tgt);
  }

  tgts_.type = MissionType::ELLIPSE_DET;
}

LandingMarkerDetectorBase::LandingMarkerDetectorBase()
{
  // this->_params_loaded = false;
  // std::string params_fn = _get_home() + SV_ROOT_DIR + "sv_algorithm_params.json";
  // if (_is_file_exist(params_fn))
  //   this->loadAlgorithmParams(params_fn);
  setupImpl();
}


LandingMarkerDetectorBase::~LandingMarkerDetectorBase()
{

}

bool LandingMarkerDetectorBase::isParamsLoaded()
{
  return this->_params_loaded;
}
int LandingMarkerDetectorBase::getMaxCandidates()
{
  return this->_max_candidates;
}
std::vector<std::string> LandingMarkerDetectorBase::getLabelsNeed()
{
  return this->_labels_need;
}


void LandingMarkerDetectorBase::detect(cv::Mat img_, TargetsInFrame& tgts_)
{
  if (!_params_loaded)
  {
    this->_load();
    this->_loadLabels();
    _params_loaded = true;
  }

  float fMaxCenterDistance = sqrt(float(img_.cols*img_.cols + img_.rows*img_.rows)) * this->_max_center_distance_ratio;
  _ed->SetMCD(fMaxCenterDistance);
  std::vector<yaed::Ellipse> ellsCned;
  _ed->Detect(img_, ellsCned);
  
  tgts_.setSize(img_.cols, img_.rows);
  tgts_.setFOV(this->fov_x, this->fov_y);
  auto t1 = std::chrono::system_clock::now();
  tgts_.setFPS(1000.0 / std::chrono::duration_cast<std::chrono::milliseconds>(t1 - this->_t0).count());
  this->_t0 = std::chrono::system_clock::now();
  tgts_.setTimeNow();
  
  static std::vector<std::string> s_label2str = {"neg", "h", "x", "1", "2", "3", "4", "5", "6", "7", "8"};
  int cand_cnt = 0;
  std::vector<cv::Mat> input_rois;
  while (cand_cnt < this->_max_candidates && ellsCned.size() > cand_cnt)
  {
    yaed::Ellipse e = ellsCned[cand_cnt++];

    cv::Rect rect;
    e.GetRectangle(rect);
    int x1 = rect.x;
    int y1 = rect.y;
    int x2 = rect.x + rect.width;
    int y2 = rect.y + rect.height;
    if (x1 < 0)  x1 = 0;
    if (y1 < 0)  y1 = 0;
    if (x2 > img_.cols - 1)  x2 = img_.cols - 1;
    if (y2 > img_.rows - 1)  y2 = img_.rows - 1;
    if (x2 - x1 < 5 || y2 - y1 < 5)  continue;
    rect.x = x1;
    rect.y = y1;
    rect.width = x2 - x1;
    rect.height = y2 - y1;

    cv::Mat e_roi = img_(rect);
    cv::resize(e_roi, e_roi, cv::Size(32, 32));
    
    input_rois.push_back(e_roi);
  }

  std::vector<int> output_labels;
  roiCNN(input_rois, output_labels);
  if (input_rois.size() != output_labels.size())
    throw std::runtime_error("SpireCV (106) input_rois.size() != output_labels.size()");

  for (int i=0; i<output_labels.size(); i++)
  {
    int label = output_labels[i];
    bool need = false;
    for (int j=0; j<_labels_need.size(); j++)
    {
      if (this->_labels_need[j] == s_label2str[label])
      {
        need = true;
      }
    }
    if (!need)  label = 0;

    yaed::Ellipse e = ellsCned[i];
    if (label > 0)
    {
      Target tgt;
      tgt.setEllipse(e.xc_, e.yc_, e.a_, e.b_, e.rad_, e.score_, tgts_.width, tgts_.height, this->camera_matrix, this->_radius_in_meter);
      tgt.setCategory(s_label2str[label], label);
      tgts_.targets.push_back(tgt);
    }
  }

  tgts_.type = MissionType::LANDMARK_DET;
}
bool LandingMarkerDetectorBase::setupImpl()
{
  return false;
}
void LandingMarkerDetectorBase::roiCNN(std::vector<cv::Mat>& input_rois_, std::vector<int>& output_labels_)
{

}


void LandingMarkerDetectorBase::_loadLabels()
{
  JsonValue all_value;
  JsonAllocator allocator;
  _load_all_json(this->alg_params_fn, all_value, allocator);
  
  JsonValue landing_params_value;
  _parser_algorithm_params("LandingMarkerDetector", all_value, landing_params_value);
  
  for (auto i : landing_params_value) {
    if ("labels" == std::string(i->key) && i->value.getTag() == JSON_ARRAY) {
      for (auto j : i->value) {
        this->_labels_need.push_back(j->value.toString());
      }
    }
    else if ("maxCandidates" == std::string(i->key)) {
      this->_max_candidates = i->value.toNumber();
      // std::cout << "maxCandidates: " << this->_max_candidates << std::endl;
    }
  }
  setupImpl();
}


void EllipseDetector::detectAllInDirectory(std::string input_img_dir_, std::string output_json_dir_)
{
  if (!_params_loaded)
  {
    this->_load();
    _params_loaded = true;
  }
  
  std::vector<std::string> files;
  yaed::_list_dir(input_img_dir_, files, "jpg");
  
  for (size_t i=0; i<files.size(); i++)
  {
    std::string fn = input_img_dir_ + "/" + files[i];
    cv::Mat img = cv::imread(fn);
    std::cout << fn << std::endl;
    std::string label_str = "ellipse"; 
    
    if (files[i].size() > 9 && files[i].substr(8, 1) == "_")
    {
      label_str = files[i].substr(9, 1);
      std::cout << label_str << std::endl;
    }
    
    cv::Mat resultImage = img.clone();
    
    float fMaxCenterDistance = sqrt(float(img.cols*img.cols + img.rows*img.rows)) * this->_max_center_distance_ratio;
    _ed->SetMCD(fMaxCenterDistance);
    std::vector<yaed::Ellipse> ellsCned;
    _ed->Detect(img, ellsCned);
    
    std::ofstream ofs(output_json_dir_ + "/" + files[i] + ".json");
    std::string inst_str = "";
    int j = 0;
    char buf[1024*32];
    for (yaed::Ellipse e : ellsCned)
    {
      cv::Rect rect;
      e.GetRectangle(rect);
      cv::rectangle(resultImage, rect, (0,0,255), 1);

      sprintf(buf, "{\"category_name\":\"%s\",\"bbox\":[%d,%d,%d,%d],\"area\":%d,\"score\":%.3f}", label_str.c_str(), rect.x, rect.y, rect.width, rect.height, rect.width*rect.height, e.score_);
      inst_str += std::string(buf);
      if (j < ellsCned.size() - 1)
        inst_str += ",";
        // ofs << e.xc_ << "," << e.yc_ << "," << e.a_ << "," << e.b_ << "," << e.rad_ << "," << e.score_ << std::endl;
      j++;
    }

    sprintf(buf, "{\"file_name\":\"%s\",\"height\":%d,\"width\":%d,\"annos\":[%s]}", files[i].c_str(), img.rows, img.cols, inst_str.c_str());
    ofs << buf << std::endl;
    ofs.close();

    cv::imshow("img", resultImage);
    cv::waitKey(100);
  }
}

void EllipseDetector::_load()
{
  JsonValue all_value;
  JsonAllocator allocator;
  _load_all_json(this->alg_params_fn, all_value, allocator);

  JsonValue ell_params_value;
  _parser_algorithm_params("EllipseDetector", all_value, ell_params_value);
  
  cv::Size szPreProcessingGaussKernel;
  double   dPreProcessingGaussSigma;
  float    fThPosition;
  float    fMaxCenterDistance;
  int      iMinEdgeLength;
  float    fMinOrientedRectSide;
  float    fDistanceToEllipseContour;
  float    fMinScore;
  float    fMinReliability;
  int      iNs;
  double   dPercentNe;
  float    fT_CNC;
  float    fT_TCN_L;
  float    fT_TCN_P;
  float    fThre_R;
  
  for (auto i : ell_params_value) {
    if ("preProcessingGaussKernel" == std::string(i->key)) {
      int sigma = i->value.toNumber();
      szPreProcessingGaussKernel = cv::Size(sigma, sigma);
      // std::cout << "preProcessingGaussKernel: " << sigma << std::endl;
    }
    else if ("preProcessingGaussSigma" == std::string(i->key)) {
      dPreProcessingGaussSigma = i->value.toNumber();
      // std::cout << "preProcessingGaussSigma: " << dPreProcessingGaussSigma << std::endl;
    }
    else if ("thPosition" == std::string(i->key)) {
      fThPosition = i->value.toNumber();
      // std::cout << "thPosition: " << fThPosition << std::endl;
    }
    else if ("maxCenterDistance" == std::string(i->key)) {
      this->_max_center_distance_ratio = i->value.toNumber();
      fMaxCenterDistance = sqrt(float(this->image_width*this->image_width + this->image_height*this->image_height)) * this->_max_center_distance_ratio;
      // std::cout << "maxCenterDistance: " << this->_max_center_distance_ratio << std::endl;
    }
    else if ("minEdgeLength" == std::string(i->key)) {
      iMinEdgeLength = i->value.toNumber();
      // std::cout << "minEdgeLength: " << iMinEdgeLength << std::endl;
    }
    else if ("minOrientedRectSide" == std::string(i->key)) {
      fMinOrientedRectSide = i->value.toNumber();
      // std::cout << "minOrientedRectSide: " << fMinOrientedRectSide << std::endl;
    }
    else if ("distanceToEllipseContour" == std::string(i->key)) {
      fDistanceToEllipseContour = i->value.toNumber();
      // std::cout << "distanceToEllipseContour: " << fDistanceToEllipseContour << std::endl;
    }
    else if ("minScore" == std::string(i->key)) {
      fMinScore = i->value.toNumber();
      // std::cout << "minScore: " << fMinScore << std::endl;
    }
    else if ("minReliability" == std::string(i->key)) {
      fMinReliability = i->value.toNumber();
      // std::cout << "minReliability: " << fMinReliability << std::endl;
    }
    else if ("ns" == std::string(i->key)) {
      iNs = i->value.toNumber();
      // std::cout << "ns: " << iNs << std::endl;
    }
    else if ("percentNe" == std::string(i->key)) {
      dPercentNe = i->value.toNumber();
      // std::cout << "percentNe: " << dPercentNe << std::endl;
    }
    else if ("T_CNC" == std::string(i->key)) {
      fT_CNC = i->value.toNumber();
      // std::cout << "T_CNC: " << fT_CNC << std::endl;
    }
    else if ("T_TCN_L" == std::string(i->key)) {
      fT_TCN_L = i->value.toNumber();
      // std::cout << "T_TCN_L: " << fT_TCN_L << std::endl;
    }
    else if ("T_TCN_P" == std::string(i->key)) {
      fT_TCN_P = i->value.toNumber();
      // std::cout << "T_TCN_P: " << fT_TCN_P << std::endl;
    }
    else if ("thRadius" == std::string(i->key)) {
      fThre_R = i->value.toNumber();
      // std::cout << "thRadius: " << fThre_R << std::endl;
    }
    else if ("radiusInMeter" == std::string(i->key)) {
      this->_radius_in_meter = i->value.toNumber();
      // std::cout << "radiusInMeter: " << this->_radius_in_meter << std::endl;
    }
  }
  
  if (_ed) { delete _ed; _ed = NULL; }
  _ed = new yaed::EllipseDetector;
  _ed->SetParameters(szPreProcessingGaussKernel, dPreProcessingGaussSigma, fThPosition, fMaxCenterDistance, iMinEdgeLength, fMinOrientedRectSide, fDistanceToEllipseContour, fMinScore, fMinReliability, iNs, dPercentNe, fT_CNC, fT_TCN_L, fT_TCN_P, fThre_R);
}


SingleObjectTrackerBase::SingleObjectTrackerBase()
{  
  this->_params_loaded = false;
}
SingleObjectTrackerBase::~SingleObjectTrackerBase()
{

}
bool SingleObjectTrackerBase::isParamsLoaded()
{
  return this->_params_loaded;
}
std::string SingleObjectTrackerBase::getAlgorithm()
{
  return this->_algorithm;
}
int SingleObjectTrackerBase::getBackend()
{
  return this->_backend;
}
int SingleObjectTrackerBase::getTarget()
{
  return this->_target;
}

void SingleObjectTrackerBase::warmUp()
{
  cv::Mat testim = cv::Mat::zeros(640, 480, CV_8UC3);
  this->init(testim, cv::Rect(10, 10, 100, 100));
  TargetsInFrame testtgts(0);
  this->track(testim, testtgts);
}
void SingleObjectTrackerBase::init(cv::Mat img_, const cv::Rect& bounding_box_)
{
  if (!this->_params_loaded)
  {
    this->_load();
    this->_params_loaded = true;
  }

  if (bounding_box_.width < 4 || bounding_box_.height < 4)
  {
    throw std::runtime_error("SpireCV (106) Tracking box size < (4, 4), too small!");
  }
  if (bounding_box_.x < 0 || bounding_box_.y < 0 || bounding_box_.x + bounding_box_.width > img_.cols || bounding_box_.y + bounding_box_.height > img_.rows)
  {
    throw std::runtime_error("SpireCV (106) Tracking box not in the Input Image!");
  }
  
  initImpl(img_, bounding_box_);
}
void SingleObjectTrackerBase::track(cv::Mat img_, TargetsInFrame& tgts_)
{
  Rect rect;
  bool ok = trackImpl(img_, rect);
  
  tgts_.setSize(img_.cols, img_.rows);
  tgts_.setFOV(this->fov_x, this->fov_y);
  auto t1 = std::chrono::system_clock::now();
  tgts_.setFPS(1000.0 / std::chrono::duration_cast<std::chrono::milliseconds>(t1 - this->_t0).count());
  this->_t0 = std::chrono::system_clock::now();
  tgts_.setTimeNow();
  
  if (ok)
  {
    Target tgt;
    tgt.setBox(rect.x, rect.y, rect.x+rect.width, rect.y+rect.height, img_.cols, img_.rows);
    tgt.setTrackID(1);
    tgt.setLOS(tgt.cx, tgt.cy, this->camera_matrix, img_.cols, img_.rows);
    tgts_.targets.push_back(tgt);
  }

  tgts_.type = MissionType::TRACKING;
}
bool SingleObjectTrackerBase::setupImpl()
{
  return false;
}
void SingleObjectTrackerBase::initImpl(cv::Mat img_, const cv::Rect& bounding_box_)
{

}
bool SingleObjectTrackerBase::trackImpl(cv::Mat img_, cv::Rect& output_bbox_)
{
  return false;
}
void SingleObjectTrackerBase::_load()
{
  JsonValue all_value;
  JsonAllocator allocator;
  _load_all_json(this->alg_params_fn, all_value, allocator);

  JsonValue tracker_params_value;
  _parser_algorithm_params("SingleObjectTracker", all_value, tracker_params_value);
  
  for (auto i : tracker_params_value) {
    if ("algorithm" == std::string(i->key)) {
      this->_algorithm = i->value.toString();
      std::cout << "algorithm: " << this->_algorithm << std::endl;
    }
    else if ("backend" == std::string(i->key)) {
      this->_backend = i->value.toNumber();
    }
    else if ("target" == std::string(i->key)) {
      this->_target = i->value.toNumber();
    }
  }

  setupImpl();
}


CommonObjectDetectorBase::CommonObjectDetectorBase() // : CameraAlgorithm()
{
  this->_params_loaded = false;
  // std::cout << "CommonObjectDetectorBase->_params_loaded: " << this->_params_loaded << std::endl;
}
CommonObjectDetectorBase::~CommonObjectDetectorBase()
{

}

bool CommonObjectDetectorBase::isParamsLoaded()
{
  return this->_params_loaded;
}
std::string CommonObjectDetectorBase::getDataset()
{
  return this->_dataset;
}
std::vector<std::string> CommonObjectDetectorBase::getClassNames()
{
  return this->_class_names;
}
std::vector<double> CommonObjectDetectorBase::getClassWs()
{
  return this->_class_ws;
}
std::vector<double> CommonObjectDetectorBase::getClassHs()
{
  return this->_class_hs;
}
int CommonObjectDetectorBase::getInputH()
{
  return this->_input_h;
}
int CommonObjectDetectorBase::getInputW()
{
  return this->_input_w;
}
int CommonObjectDetectorBase::getClassNum()
{
  return this->_n_classes;
}
int CommonObjectDetectorBase::getOutputSize()
{
  return this->_output_size;
}
double CommonObjectDetectorBase::getThrsNms()
{
  return this->_thrs_nms;
}
double CommonObjectDetectorBase::getThrsConf()
{
  return this->_thrs_conf;
}
int CommonObjectDetectorBase::useWidthOrHeight()
{
  return this->_use_width_or_height;
}
bool CommonObjectDetectorBase::withSegmentation()
{
  return this->_with_segmentation;
}
void CommonObjectDetectorBase::setInputH(int h_)
{
  this->_input_h = h_;
}
void CommonObjectDetectorBase::setInputW(int w_)
{
  this->_input_w = w_;
}

void CommonObjectDetectorBase::warmUp()
{
  cv::Mat testim = cv::Mat::zeros(640, 480, CV_8UC3);
  TargetsInFrame testtgts(0);
  this->detect(testim, testtgts);
}

void CommonObjectDetectorBase::detect(cv::Mat img_, TargetsInFrame& tgts_, Box* roi_, int img_w_, int img_h_)
{
  if (!this->_params_loaded)
  {
    this->_load();
    this->_params_loaded = true;
  }

  if (nullptr != roi_ && img_w_ > 0 && img_h_ > 0)
    tgts_.setSize(img_w_, img_h_);
  else
    tgts_.setSize(img_.cols, img_.rows);

  tgts_.setFOV(this->fov_x, this->fov_y);
  auto t1 = std::chrono::system_clock::now();
  tgts_.setFPS(1000.0 / std::chrono::duration_cast<std::chrono::milliseconds>(t1 - this->_t0).count());
  this->_t0 = std::chrono::system_clock::now();
  tgts_.setTimeNow();

  std::vector<float> boxes_x;
  std::vector<float> boxes_y;
  std::vector<float> boxes_w;
  std::vector<float> boxes_h;
  std::vector<int> boxes_label;
  std::vector<float> boxes_score;
  std::vector<cv::Mat> boxes_seg;
  detectImpl(img_, boxes_x, boxes_y, boxes_w, boxes_h, boxes_label, boxes_score, boxes_seg);

  size_t n_objs = boxes_x.size();
  if (n_objs != boxes_y.size() || n_objs != boxes_w.size() || n_objs != boxes_h.size() || n_objs != boxes_label.size() || n_objs != boxes_score.size())
    throw std::runtime_error("SpireCV (106) Error in detectImpl(), Vector Size Not Equal!");

  if (this->_with_segmentation && n_objs != boxes_seg.size())
    throw std::runtime_error("SpireCV (106) Error in detectImpl(), Vector Size Not Equal!");
  
  for (int j=0; j<n_objs; j++)
  {
    int ox = int(round(boxes_x[j]));
    int oy = int(round(boxes_y[j]));
    int ow = int(round(boxes_w[j]));
    int oh = int(round(boxes_h[j]));
    if (ox < 0) ox = 0;
    if (oy < 0) oy = 0;
    if (ox + ow >= img_.cols) ow = img_.cols - ox - 1;
    if (oy + oh >= img_.rows) oh = img_.rows - oy - 1;
    if (ow > 5 && oh > 5)
    {
      Target tgt;
      if (nullptr != roi_ && img_w_ > 0 && img_h_ > 0)
        tgt.setBox(roi_->x1 + ox, roi_->y1 + oy, roi_->x1 + ox + ow, roi_->y1 + oy + oh, img_w_, img_h_);
      else
        tgt.setBox(ox, oy, ox+ow, oy+oh, img_.cols, img_.rows);

      int cat_id = boxes_label[j];
      tgt.setCategory(this->_class_names[cat_id], cat_id);
      if (nullptr != roi_ && img_w_ > 0 && img_h_ > 0)
        tgt.setLOS(tgt.cx, tgt.cy, this->camera_matrix, img_w_, img_h_);
      else
        tgt.setLOS(tgt.cx, tgt.cy, this->camera_matrix, img_.cols, img_.rows);
      tgt.score = boxes_score[j];
      if (this->_use_width_or_height == 0)
      {
        double z = this->camera_matrix.at<double>(0, 0) * this->_class_ws[cat_id] / ow;
        double x = tan(tgt.los_ax / SV_RAD2DEG) * z;
        double y = tan(tgt.los_ay / SV_RAD2DEG) * z;
        tgt.setPosition(x, y, z);
      }
      else if (this->_use_width_or_height == 1)
      {
        double z = this->camera_matrix.at<double>(1, 1) * this->_class_hs[cat_id] / oh;
        double x = tan(tgt.los_ax / SV_RAD2DEG) * z;
        double y = tan(tgt.los_ay / SV_RAD2DEG) * z;
        tgt.setPosition(x, y, z);
      }

      if (this->_with_segmentation)
      {
        cv::Mat mask_j = boxes_seg[j].clone();
        int maskh = mask_j.rows, maskw = mask_j.cols;
        assert(maskh == maskw);

        if (img_.cols > img_.rows)
        {
          int cut_h = (int)round((img_.rows * 1. / img_.cols) * maskh);
          int gap_h = (int)round((maskh - cut_h) / 2.);
          mask_j = mask_j.rowRange(gap_h, gap_h + cut_h);
        }
        else if (img_.cols < img_.rows)
        {
          int cut_w = (int)round((img_.cols * 1. / img_.rows) * maskh);
          int gap_w = (int)round((maskh - cut_w) / 2.);
          mask_j = mask_j.colRange(gap_w, gap_w + cut_w);
        }

        if (nullptr != roi_ && img_w_ > 0 && img_h_ > 0)
        {
          cv::resize(mask_j, mask_j, cv::Size(img_.cols, img_.rows));

          cv::Mat mask_out = cv::Mat::zeros(img_h_, img_w_, CV_32FC1);
          mask_j.copyTo(mask_out(cv::Rect(roi_->x1, roi_->y1, mask_j.cols, mask_j.rows)));

          tgt.setMask(mask_out);
        }
        else
        {
          tgt.setMask(mask_j);
        }
      }

      tgts_.targets.push_back(tgt);
    }
  }

  tgts_.type = MissionType::COMMON_DET;
}

bool CommonObjectDetectorBase::setupImpl()
{
  return false;
}

void CommonObjectDetectorBase::detectImpl(
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

}

void CommonObjectDetectorBase::_load()
{
  JsonValue all_value;
  JsonAllocator allocator;
  _load_all_json(this->alg_params_fn, all_value, allocator);

  JsonValue detector_params_value;
  _parser_algorithm_params("CommonObjectDetector", all_value, detector_params_value);

  // std::cout << _get_home() + "/.spire/" << std::endl;
  // stuff we know about the network and the input/output blobs
  this->_input_h = 640;
  this->_input_w = 640;
  this->_n_classes = 1;
  this->_thrs_nms = 0.6;
  this->_thrs_conf = 0.4;
  this->_use_width_or_height = 0;
  
  for (auto i : detector_params_value) {

    if ("dataset" == std::string(i->key)) {
      this->_dataset = i->value.toString();
      std::cout << "dataset: " << this->_dataset << std::endl;
    }
    else if ("inputSize" == std::string(i->key)) {
      // std::cout << "inputSize (old, new): " << this->_input_w << ", " << i->value.toNumber() << std::endl;
      this->_input_w = i->value.toNumber();
      if (this->_input_w != 640 && this->_input_w != 1280)
      {
        throw std::runtime_error("SpireCV (106) inputSize should be 640 or 1280!");
      }
      this->_input_h = this->_input_w;
    }
    else if ("nmsThrs" == std::string(i->key)) {
      // std::cout << "nmsThrs (old, new): " << this->_thrs_nms << ", " << i->value.toNumber() << std::endl;
      this->_thrs_nms = i->value.toNumber();
    }
    else if ("scoreThrs" == std::string(i->key)) {
      // std::cout << "scoreThrs (old, new): " << this->_thrs_conf << ", " << i->value.toNumber() << std::endl;
      this->_thrs_conf = i->value.toNumber();
    }
    else if ("useWidthOrHeight" == std::string(i->key)) {
      // std::cout << "useWidthOrHeight (old, new): " << this->_use_width_or_height << ", " << i->value.toNumber() << std::endl;
      this->_use_width_or_height = i->value.toNumber();
    }
    else if ("withSegmentation" == std::string(i->key)) {
      bool json_tf = false;
      if (i->value.getTag() == JSON_TRUE)  json_tf = true;
      this->_with_segmentation = json_tf;
    }
    else if ("dataset" + this->_dataset == std::string(i->key)) {
      if (i->value.getTag() == JSON_OBJECT) {
        for (auto j : i->value) {
          // std::cout << j->key << std::endl;
          _class_names.push_back(std::string(j->key));
          if (j->value.getTag() == JSON_ARRAY)
          {
            int k_cnt = 0;
            for (auto k : j->value) {
              // std::cout << k->value.toNumber() << std::endl;
              if (k_cnt == 0)      _class_ws.push_back(k->value.toNumber());
              else if (k_cnt == 1) _class_hs.push_back(k->value.toNumber());
              k_cnt ++;
            }
          }
        }
      }
    }
  }

  setupImpl();

}













}

