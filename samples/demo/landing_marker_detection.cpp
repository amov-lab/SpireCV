#include <iostream>
#include <string>
// 包含SpireCV SDK头文件
#include <sv_world.h>

using namespace std;

int main(int argc, char *argv[]) {
  // 实例化 圆形降落标志 检测器类
  sv::LandingMarkerDetector lmd;
  // 手动导入相机参数，如果使用Amov的G1等吊舱或相机，则可以忽略该步骤，将自动下载相机参数文件
  lmd.loadCameraParams(sv::get_home() + "/SpireCV/calib_webcam_640x480.yaml");

  // 打开摄像头
  sv::Camera cap;
  // cap.setWH(640, 480);
  // cap.setFps(30);
  cap.open(sv::CameraType::WEBCAM, 0);  // CameraID 0
  // 实例化OpenCV的Mat类，用于内存单帧图像
  cv::Mat img;
  int frame_id = 0;
  while (1)
  {
    // 实例化SpireCV的 单帧检测结果 接口类 TargetsInFrame
    sv::TargetsInFrame tgts(frame_id++);
    // 读取一帧图像到img
    cap.read(img);
    cv::resize(img, img, cv::Size(lmd.image_width, lmd.image_height));

    // 执行 降落标志 检测
    lmd.detect(img, tgts);
    // 可视化检测结果，叠加到img上
    sv::drawTargetsInFrame(img, tgts);

    // 控制台打印 降落标志 检测结果
    printf("Frame-[%d]\n", frame_id);
    // 打印当前检测的FPS
    printf("  FPS = %.2f\n", tgts.fps);
    // 打印当前相机的视场角（degree）
    printf("  FOV (fx, fy) = (%.2f, %.2f)\n", tgts.fov_x, tgts.fov_y);
    // 打印当前输入图像的像素宽度和高度
    printf("  Frame Size (width, height) = (%d, %d)\n", tgts.width, tgts.height);
    for (int i=0; i<tgts.targets.size(); i++)
    {
      printf("Frame-[%d], Marker-[%d]\n", frame_id, i);
      // 打印每个 降落标志 的中心位置，cx，cy的值域为[0, 1]，以及cx，cy的像素值
      printf("  Marker Center (cx, cy) = (%.3f, %.3f), in Pixels = ((%d, %d))\n",
        tgts.targets[i].cx, tgts.targets[i].cy,
        int(tgts.targets[i].cx * tgts.width),
        int(tgts.targets[i].cy * tgts.height));
      // 打印每个 降落标志 的外接矩形框的宽度、高度，w，h的值域为(0, 1]，以及w，h的像素值
      printf("  Marker Size (w, h) = (%.3f, %.3f), in Pixels = ((%d, %d))\n",
        tgts.targets[i].w, tgts.targets[i].h,
        int(tgts.targets[i].w * tgts.width),
        int(tgts.targets[i].h * tgts.height));
      // 打印每个 降落标志 的置信度
      printf("  Marker Score = %.3f\n", tgts.targets[i].score);
      // 打印每个 降落标志 的类别，字符串类型，"h"、"x"、"1"、"2"、"3"...
      printf("  Marker Category = %s, Category ID = %d\n", tgts.targets[i].category.c_str(), tgts.targets[i].category_id);
      // 打印每个 降落标志 的视线角，跟相机视场相关
      printf("  Marker Line-of-sight (ax, ay) = (%.3f, %.3f)\n", tgts.targets[i].los_ax, tgts.targets[i].los_ay);
      // 打印每个 降落标志 的3D位置（在相机坐标系下），跟降落标志实际半径、相机参数相关
      printf("  Marker Position = (x, y, z) = (%.3f, %.3f, %.3f)\n", tgts.targets[i].px, tgts.targets[i].py, tgts.targets[i].pz);
    }
    
    // 显示检测结果img
    cv::imshow("img", img);
    cv::waitKey(10);
  }

  return 0;
}
