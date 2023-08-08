#include <iostream>
#include <string>
// 包含SpireCV SDK头文件
#include <sv_world.h>

using namespace std;
using namespace sv;

int main(int argc, char *argv[])
{
  // 实例化 color line detection 检测器类
  sv::ColorLineDetector cld;
  // 手动导入相机参数，如果使用Amov的G1等吊舱或相机，则可以忽略该步骤，将自动下载相机参数文件
  cld.loadCameraParams(sv::get_home() + "/SpireCV/calib_webcam_640x480.yaml");

  // 打开摄像头
  sv::Camera cap;
  cap.setWH(640, 480);
  // cap.setFps(30);
  cap.open(sv::CameraType::WEBCAM, 0); // CameraID 0
  // 实例化OpenCV的Mat类，用于内存单帧图像
  cv::Mat img;
  int frame_id = 0;
  while (1)
  {
    // 实例化SpireCV的 单帧检测结果 接口类 TargetsInFrame
    sv::TargetsInFrame tgts(frame_id++);
    // 读取一帧图像到img
    cap.read(img);
    cv::resize(img, img, cv::Size(cld.image_width, cld.image_height));

    // 执行 color line detection 检测
    cld.detect(img, tgts);
    // 可视化检测结果，叠加到img上
    sv::drawTargetsInFrame(img, tgts);

    // 控制台打印 color line detection 检测结果
    printf("Frame-[%d]\n", frame_id);
    // 打印当前检测的FPS
    printf("  FPS = %.2f\n", tgts.fps);
    // 打印当前相机的视场角（degree）
    printf("  FOV (fx, fy) = (%.2f, %.2f)\n", tgts.fov_x, tgts.fov_y);
    // 打印当前输入图像的像素宽度和高度
    printf("  Frame Size (width, height) = (%d, %d)\n", tgts.width, tgts.height);
    for (int i = 0; i < tgts.targets.size(); i++)
    {

      // 打印每个 color_line 的中心位置，cx，cy的值域为[0, 1]，以及cx，cy的像素值
      printf("  Color Line detect Center (cx, cy) = (%.3f, %.3f), in Pixels = ((%d, %d))\n",
             tgts.targets[i].cx, tgts.targets[i].cy,
             int(tgts.targets[i].cx * tgts.width),
             int(tgts.targets[i].cy * tgts.height));

      // 打印每个color_line的x_方向反正切值，跟相机视场相关
      printf("  Color Line detect Line-of-sight (ax, ay) = (%.3f, %.3f)\n", tgts.targets[i].los_ax, tgts.targets[i].los_ay);
    }

    // 显示检测结果img
    cv::imshow("img", img);
    cv::waitKey(10);
  }

  return 0;
}
