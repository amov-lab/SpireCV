#include <iostream>
#include <string>
// 包含SpireCV SDK头文件
#include <sv_world.h>
// #include "gimbal_tools.hpp"

using namespace std;

// 云台
// sampleGimbal gimbal;

// 定义窗口名称
static const std::string RGB_WINDOW = "Image window";
// 鼠标点击事件的回调函数
void onMouse(int event, int x, int y, int, void *);
// 定义吊舱
sv::Gimbal *gimbal;
// 吊舱状态获取回调函数
void GimableCallback(double &frame_ang_r, double &frame_ang_p, double &frame_ang_y,
                     double &imu_ang_r, double &imu_ang_p, double &imu_ang_y,
                     double &fov_x, double &fov_y);

// 使用KP控制器实现吊舱追踪
void gimbalTrack(double xOffset, double yOffset)
{
  gimbal->setAngleRateEuler(0, yOffset * 150.0f, xOffset * 150.0f);
}

// 停止吊舱追踪
void gimbalNoTrack(void)
{
  gimbal->setAngleRateEuler(0, 0, 0);
}

int main(int argc, char *argv[])
{
  // 实例化吊舱
  gimbal = new sv::Gimbal(sv::GimbalType::G1, sv::GimbalLink::SERIAL);
  // 使用 /dev/ttyUSB0 作为控制串口
  gimbal->setSerialPort("/dev/ttyUSB0");
  // 以GimableCallback作为状态回调函数启用吊舱控制
  gimbal->open(GimableCallback);
  // 定义相机
  sv::Camera cap;
  // 设置相机流媒体地址为 192.168.2.64
  cap.setIp("192.168.2.64");
  // 设置获取画面分辨率为720P
  cap.setWH(1280, 720);
  // 设置视频帧率为30帧
  cap.setFps(30);
  // 开启相机
  cap.open(sv::CameraType::G1);

  // 定义一个窗口 才可以在上面操作
  cv::namedWindow(RGB_WINDOW);
  // 设置窗口操作回调函数，该函数实现吊舱控制
  cv::setMouseCallback(RGB_WINDOW, onMouse, 0);
  // 实例化 圆形降落标志 检测器类
  sv::LandingMarkerDetector lmd;
  // 手动导入相机参数，如果使用Amov的G1等吊舱或相机，则可以忽略该步骤，将自动下载相机参数文件
  lmd.loadCameraParams(sv::get_home() + "/SpireCV/calib_webcam_1280x720.yaml");

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
    for (int i = 0; i < tgts.targets.size(); i++)
    {
      // 仅追踪 X 类型的标靶
      if (tgts.targets[i].category_id == 2)
      {
        gimbalTrack(tgts.targets[0].cx - 0.5f, tgts.targets[0].cy - 0.5f);
      }

      printf("Frame-[%d], Marker-[%d]\n", frame_id, i);
      // 打印每个 降落标志 的中心位置，cx，cy的值域为[0, 1]
      printf("  Marker Center (cx, cy) = (%.3f, %.3f)\n", tgts.targets[i].cx, tgts.targets[i].cy);
      // 打印每个 降落标志 的外接矩形框的宽度、高度，w，h的值域为(0, 1]
      printf("  Marker Size (w, h) = (%.3f, %.3f)\n", tgts.targets[i].w, tgts.targets[i].h);
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
    cv::imshow(RGB_WINDOW, img);
    cv::waitKey(10);
  }

  return 0;
}

void onMouse(int event, int x, int y, int, void *)
{
  if (event == cv::EVENT_LBUTTONDOWN)
  {
    gimbal->setAngleRateEuler(0, 0.02 * (y - 720 / 2), 0.02 * (x - 1280 / 2));
  }
  if (event == cv::EVENT_RBUTTONDOWN)
  {
    gimbalNoTrack();
    gimbal->setHome();
  }
}

void GimableCallback(double &frame_ang_r, double &frame_ang_p, double &frame_ang_y,
                     double &imu_ang_r, double &imu_ang_p, double &imu_ang_y,
                     double &fov_x, double &fov_y)
{
  static int count = 0;
  if (count == 25)
  {
    std::cout << "GIMBAL_CMD_RCV_POS" << std::endl;
    std::cout << "=============================================" << std::endl;
    std::cout << "HALL_yaw:" << frame_ang_y << " " << std::endl;
    std::cout << "HALL_roll:" << frame_ang_r << " " << std::endl;
    std::cout << "HALL_pitch:" << frame_ang_p << " " << std::endl;
    std::cout << "GYRO_yaw:" << imu_ang_y << " " << std::endl;
    std::cout << "GYRO_roll:" << imu_ang_r << " " << std::endl;
    std::cout << "GYRO_pitch:" << imu_ang_p << " " << std::endl;
    count = 0;
  }
  count++;
}