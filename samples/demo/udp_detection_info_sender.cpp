#include <iostream>
#include <string>
// 包含SpireCV SDK头文件
#include <sv_world.h>

using namespace std;

int main(int argc, char *argv[]) {
  // 实例化Aruco检测器类
  sv::ArucoDetector ad;
  // 手动导入相机参数，如果使用Amov的G1等吊舱或相机，则可以忽略该步骤，将自动下载相机参数文件
  ad.loadCameraParams("/home/amov/SpireCV/calib_webcam_640x480.yaml");
  
  // 打开摄像头
  sv::Camera cap;
  cap.setWH(640, 480);
  cap.setFps(30);
  cap.open(sv::CameraType::WEBCAM, 0);  // CameraID 0

  sv::UDPServer udp;
  // 实例化OpenCV的Mat类，用于内存单帧图像
  cv::Mat img;
  int frame_id = 0;
  while (1)
  {
    // 实例化SpireCV的 单帧检测结果 接口类 TargetsInFrame
    sv::TargetsInFrame tgts(frame_id++);
    // 读取一帧图像到img
    cap.read(img);

    // 执行Aruco二维码检测
    ad.detect(img, tgts);

    tgts.has_pod_info = true;
    tgts.pod_patch = 1;
    tgts.pod_roll = 2;
    tgts.pod_yaw = 3;
    tgts.has_uav_pos = true;
    tgts.longitude = 1.1234567;
    tgts.latitude = 2.2345678;
    tgts.altitude = 3.3456789;
    tgts.has_uav_vel = true;
    tgts.uav_vx = 4;
    tgts.uav_vy = 5;
    tgts.uav_vz = 6;
    tgts.has_ill = true;
    tgts.illumination = 7;

    // www.write(img, tgts);
    udp.send(tgts);
    // 可视化检测结果，叠加到img上
    sv::drawTargetsInFrame(img, tgts);
    
    // 显示检测结果img
    cv::imshow("img", img);
    cv::waitKey(10);
  }

  return 0;
}
