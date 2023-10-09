#include <iostream>
#include <string>
// 包含SpireCV SDK头文件
#include <sv_world.h>

using namespace std;

// yaw roll pitch
double gimbalEulerAngle[3];
void gimableCallback(double &imu_ang_r, double &imu_ang_p, double &imu_ang_y,
                     double &frame_ang_r, double &frame_ang_p, double &frame_ang_y,
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

  gimbalEulerAngle[0] = frame_ang_y;
  gimbalEulerAngle[1] = imu_ang_r;
  gimbalEulerAngle[2] = imu_ang_p;

  count++;
}

// 定义吊舱
sv::Gimbal *gimbal;

// 定义窗口名称
static const std::string RGB_WINDOW = "Image window";

// 鼠标点击事件的回调函数
void onMouse(int event, int x, int y, int, void *);

int main(int argc, char *argv[])
{
  // 实例化吊舱
  gimbal = new sv::Gimbal(sv::GimbalType::G1, sv::GimbalLink::SERIAL);
  // 使用 /dev/ttyUSB0 作为控制串口
  gimbal->setSerialPort("/dev/ttyUSB0");
  // 以gimableCallback作为状态回调函数启用吊舱控制
  gimbal->open(gimableCallback);
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
  // 实例化Aruco检测器类
  sv::ArucoDetector ad;
  // 手动导入相机参数，如果使用Amov的G1等吊舱或相机，则可以忽略该步骤，将自动下载相机参数文件
  ad.loadCameraParams(sv::get_home() + "/SpireCV/calib_webcam_1280x720.yaml");

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
    tgts.pod_patch = gimbalEulerAngle[2];
    tgts.pod_roll = gimbalEulerAngle[1];
    tgts.pod_yaw = gimbalEulerAngle[0];

    tgts.has_uav_pos = false;
    tgts.has_uav_vel = false;
    tgts.has_ill = false;

    // www.write(img, tgts);
    udp.send(tgts);
    // 可视化检测结果，叠加到img上
    sv::drawTargetsInFrame(img, tgts);

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
    gimbal->setHome();
  }
}
