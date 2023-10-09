#include <iostream>
#include <string>
// 包含SpireCV SDK头文件
#include <sv_world.h>

using namespace std;

int main(int argc, char *argv[]) {  
  // 打开摄像头
  sv::VeriDetector veri;
  veri.loadCameraParams(sv::get_home() + "/SpireCV/calib_webcam_640x480.yaml");

  cv::VideoCapture cap1("/home/amov/Videos/com/FlyVideo_2023-09-02_11-36-00.avi");
  cv::VideoCapture cap2("/home/amov/Videos/com/FlyVideo_2023-09-02_11-41-55.avi");
  // cap.setWH(640, 480);
  // cap.setFps(30);
  //cap.open(sv::CameraType::WEBCAM, 0);  // CameraID 0
  // 实例化OpenCV的Mat类，用于内存单帧图像


  cv::Mat img1,img2;
  int frame_id = 0;
  while (1)
  {
    // 实例化SpireCV的 单帧检测结果 接口类 TargetsInFrame
    sv::TargetsInFrame tgts(frame_id++);

    // 读取一帧图像到img
    cap1.read(img1);
    cap2.read(img2);

    cv::resize(img1, img1, cv::Size(224, 224));
    cv::resize(img2, img2, cv::Size(224, 224));

    veri.detect(img1, img2, tgts);

    

    // 显示img
    // cv::imshow("img", img);
    // cv::waitKey(10);
  }

  return 0;
}
