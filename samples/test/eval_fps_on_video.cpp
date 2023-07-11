#include <iostream>
#include <string>
// 包含SpireCV SDK头文件
#include <sv_world.h>
#include <chrono>

using namespace std;

int main(int argc, char *argv[]) {
  if (argc != 4) return false;
  std::string video_fn = std::string(argv[1]);
  std::string camera_yaml_fn = std::string(argv[2]);
  std::string algorithm = std::string(argv[3]);
  // 1: ArucoDetector
  // 2: LandingMarkerDetector
  // 3: EllipseDetector
  // 41: CommonObjectDetector 640 without mask
  // 42: CommonObjectDetector 1280 without mask
  // 43: CommonObjectDetector 640 with mask
  // 51: SingleObjectTracker CSRT
  // 52: SingleObjectTracker KCF
  // 53: SingleObjectTracker SiamPRN
  // 54: SingleObjectTracker Nano

  cv::VideoCapture cap(video_fn);
  int fcount = cap.get(cv::CAP_PROP_FRAME_COUNT);
  fcount = std::min(5000, fcount);
  
  cv::Mat img;
  std::vector<cv::Mat> imgs;
  int frame_id = 0;
  for (int i=0; i<fcount; i++)
  {
    cap.read(img);
    imgs.push_back(img.clone());
    frame_id ++;
  }
  
  // 实例化Aruco检测器类
  sv::ArucoDetector ad;
  // 实例化 圆形降落标志 检测器类
  sv::LandingMarkerDetector lmd;
  // 实例化 椭圆 检测器类
  sv::EllipseDetector ed;
  // 实例化 通用目标 检测器类
  sv::CommonObjectDetector cod;
  // 实例化 框选目标跟踪类
  sv::SingleObjectTracker sot;
  // 手动导入相机参数，如果使用Amov的G1等吊舱或相机，则可以忽略该步骤，将自动下载相机参数文件
  ad.loadCameraParams(camera_yaml_fn);
  lmd.loadCameraParams(camera_yaml_fn);
  ed.loadCameraParams(camera_yaml_fn);
  cod.loadCameraParams(camera_yaml_fn);
  sot.loadCameraParams(camera_yaml_fn);
  if ("51" == algorithm)
    sot.loadAlgorithmParams(sv::get_home() + "/SpireCV/sv_algorithm_params_csrt.json");
  else if (("52" == algorithm))
    sot.loadAlgorithmParams(sv::get_home() + "/SpireCV/sv_algorithm_params_kcf.json");
  else if (("53" == algorithm))
    sot.loadAlgorithmParams(sv::get_home() + "/SpireCV/sv_algorithm_params_siamrpn.json");
  else if (("54" == algorithm))
    sot.loadAlgorithmParams(sv::get_home() + "/SpireCV/sv_algorithm_params_nano.json");
    
  if ("41" == algorithm)
    cod.loadAlgorithmParams(sv::get_home() + "/SpireCV/sv_algorithm_params_640_wo_mask.json");
  else if (("42" == algorithm))
    cod.loadAlgorithmParams(sv::get_home() + "/SpireCV/sv_algorithm_params_1280_wo_mask.json");
  else if (("43" == algorithm))
    cod.loadAlgorithmParams(sv::get_home() + "/SpireCV/sv_algorithm_params_640_w_mask.json");
  
  for (int i=0; i<10; i++)
  {
    img = imgs[0];
    cv::resize(img, img, cv::Size(ad.image_width, ad.image_height));
    
    sv::TargetsInFrame tgts(i + 1);

    // 执行Aruco二维码检测
    if ("1" == algorithm)
    ad.detect(img, tgts);
    if ("2" == algorithm)
    lmd.detect(img, tgts);
    if ("3" == algorithm)
    ed.detect(img, tgts);
    if ("41" == algorithm || "42" == algorithm || "43" == algorithm)
    cod.detect(img, tgts);
    if ("51" == algorithm || "52" == algorithm || "53" == algorithm || "54" == algorithm)
    {
      if (0 == i)
      {
        if (sot.image_width == 1920)
          sot.init(img, cv::Rect(1155, 493, 268, 442));
        else if (sot.image_width == 1280)
          sot.init(img, cv::Rect(784, 278, 191, 276));
        else // 640
          sot.init(img, cv::Rect(309, 137, 81, 238));
      }
      else
      {
        sot.track(img, tgts);
      }
    }
  }
  
  auto start = std::chrono::system_clock::now();
  for (int i=0; i<fcount; i++)
  {
    img = imgs[i];
    cv::resize(img, img, cv::Size(ad.image_width, ad.image_height));
    
    sv::TargetsInFrame tgts(i + 1);
    
    
    // 执行Aruco二维码检测
    if ("1" == algorithm)
    ad.detect(img, tgts);
    if ("2" == algorithm)
    lmd.detect(img, tgts);
    if ("3" == algorithm)
    ed.detect(img, tgts);
    if ("41" == algorithm || "42" == algorithm || "43" == algorithm)
    cod.detect(img, tgts);
    if ("51" == algorithm || "52" == algorithm || "53" == algorithm || "54" == algorithm)
    {
      if (0 == i)
      {
        if (sot.image_width == 1920)
          sot.init(img, cv::Rect(1155, 493, 268, 442));
        else if (sot.image_width == 1280)
          sot.init(img, cv::Rect(784, 278, 191, 276));
        else // 640
          sot.init(img, cv::Rect(309, 137, 81, 238));
      }
      else
      {
        sot.track(img, tgts);
      }
    }
    // 可视化检测结果，叠加到img上
    sv::drawTargetsInFrame(img, tgts);

    //cv::imshow("img", img);
    //cv::waitKey(10);
  }
  auto end = std::chrono::system_clock::now();
  
  if ("1" == algorithm)
  std::cout << "Aruco [" << camera_yaml_fn << "]" << std::endl;
  if ("2" == algorithm)
  std::cout << "Landing [" << camera_yaml_fn << "]" << std::endl;
  if ("3" == algorithm)
  std::cout << "Ellipse [" << camera_yaml_fn << "]" << std::endl;
  if ("41" == algorithm)
  std::cout << "CommonDetector 640 WO Mask [" << camera_yaml_fn << "]" << std::endl;
  if ("42" == algorithm)
  std::cout << "CommonDetector 1280 WO Mask [" << camera_yaml_fn << "]" << std::endl;
  if ("43" == algorithm)
  std::cout << "CommonDetector 640 W Mask [" << camera_yaml_fn << "]" << std::endl;
  if ("51" == algorithm)
  std::cout << "Tracking CSRT [" << camera_yaml_fn << "]" << std::endl;
  if ("52" == algorithm)
  std::cout << "Tracking KCF [" << camera_yaml_fn << "]" << std::endl;
  if ("53" == algorithm)
  std::cout << "Tracking SiamPRN [" << camera_yaml_fn << "]" << std::endl;
  if ("54" == algorithm)
  std::cout << "Tracking Nano [" << camera_yaml_fn << "]" << std::endl;
  std::cout << "FPS: " << fcount / (std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() / 1000. + 1e-4) << std::endl;
  

  return 0;
}
