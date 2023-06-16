#include <iostream>
#include <string>
// 包含SpireCV SDK头文件
#include <sv_world.h>

using namespace std;

int main(int argc, char *argv[]) {
  // 实例化 通用目标 检测器类
  sv::CommonObjectDetector cod;
  // 手动导入相机参数，如果使用Amov的G1等吊舱或相机，则可以忽略该步骤，将自动下载相机参数文件
  cod.loadCameraParams("/home/amov/SpireCV/calib_webcam_640x480.yaml");
  
  // 打开摄像头
  sv::Camera cap;
  // cap.setWH(640, 480);
  // cap.setFps(30);
  cap.open(sv::CameraType::WEBCAM, 0);  // CameraID 0
  // 实例化OpenCV的Mat类，用于内存单帧图像
  cv::Mat img;
  int frame_id = 0;

  // 实例化视频保存类
  sv::VideoWriter vw;
  // 设置保存路径"/home/amov/Videos"，保存图像尺寸（640，480），帧频25Hz，同步保存检测结果（.svj）
  vw.setup("/home/amov/Videos", cv::Size(640, 480), 25, true);

  while (1)
  {
    // 实例化SpireCV的 单帧检测结果 接口类 TargetsInFrame
    sv::TargetsInFrame tgts(frame_id++);
    // 读取一帧图像到img
    cap.read(img);
    cv::resize(img, img, cv::Size(640, 480));

    // 执行通用目标检测
    cod.detect(img, tgts);
   
    // 同步保存视频流 和 检测结果信息
    vw.write(img, tgts);

    // 可视化检测结果，叠加到img上
    sv::drawTargetsInFrame(img, tgts);
    
    // 显示检测结果img
    cv::imshow("img", img);
    cv::waitKey(10);
  }

  return 0;
}
