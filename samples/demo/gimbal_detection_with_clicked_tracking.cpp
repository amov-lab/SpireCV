#include <iostream>
#include <string>
// 包含SpireCV SDK头文件
#include <sv_world.h>
#include <map>

// 定义窗口名称
static const std::string RGB_WINDOW = "Image window";

// 实现框选逻辑的回调函数
void onMouse(int event, int x, int y, int, void *);
bool isTarck = false;
bool isStartTarck = false;
int clickX = -1, clickY = -1;

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

  // 定义一个新的窗口，可在上面进行框选操作
  cv::namedWindow(RGB_WINDOW);
  // 设置窗口操作回调函数，该函数实现整个框选逻辑
  cv::setMouseCallback(RGB_WINDOW, onMouse, 0);
  // 实例化 框选目标跟踪类
  sv::SingleObjectTracker sot;
  // 手动导入相机参数，如果使用Amov的G1等吊舱或相机，则可以忽略该步骤，将自动下载相机参数文件
  sot.loadCameraParams(sv::get_home() + "/SpireCV/confs/calib_webcam_1280x720.yaml");
  sot.loadAlgorithmParams(sv::get_home() + "/SpireCV/confs/sv_algorithm_params.json");

  sv::CommonObjectDetector cod;
  cod.loadCameraParams(sv::get_home() + "/SpireCV/confs/calib_webcam_1280x720.yaml");
  cod.loadAlgorithmParams(sv::get_home() + "/SpireCV/confs/sv_algorithm_params.json");

  // 实例化OpenCV的Mat类，用于内存单帧图像
  cv::Mat img;
  int frame_id = 0;
  sv::TargetsInFrame lastTgts(frame_id);
  while (1)
  {
    // 如果位标非法 则不能进行任何形式的追踪
    if (clickX == -1 || clickY == -1)
    {
      isStartTarck = false;
      isTarck = false;
    }

    if (isStartTarck && !isTarck)
    {
      // 使用上一帧结果初始化追踪器
      std::map<int, cv::Rect> inBoxList;

      // 计算点击事件是否位于每个目标框内
      for (int i = 0; i < lastTgts.targets.size(); i++)
      {
        int halfWidht = (lastTgts.targets[i].w * lastTgts.width) / 2;
        int halfHeight = (lastTgts.targets[i].h * lastTgts.height) / 2;
        int x = lastTgts.targets[i].cx * lastTgts.width;
        int y = lastTgts.targets[i].cy * lastTgts.height;

        int diffX = x - clickX;
        int diffY = y - clickY;

        if ((abs(diffX) < halfWidht) && (abs(diffY) < halfHeight))
        {
          std::pair<int, cv::Rect> point;
          point.first = diffX * diffX + diffY * diffY;
          point.second = cv::Rect(x - halfWidht, y - halfHeight, halfWidht * 2, halfHeight * 2);
          inBoxList.insert(point);
        }
      }

      // 取离中心点最近的目标进行跟踪
      int min = 0X7FFFFFFF;
      cv::Rect sel;
      for (auto i = inBoxList.begin(); i != inBoxList.end(); i++)
      {
        if (i->first < min)
        {
          min = i->first;
          sel = i->second;
        }
      }

      // min被赋值则存在一个目标框与点击的点重合
      if (min != 0X7FFFFFFF)
      {
        sot.init(img, sel);
        isTarck = true;
        printf("rect_sel Yes\n");
      }
      else
      {
        isTarck = false;
        printf("rect_sel No\n");
      }

      isStartTarck = false;
    }

    sv::TargetsInFrame tgts(frame_id++);
    // 读取一帧图像
    cap.read(img);

    if (!isTarck)
    {
      // 缩放图像尺寸用于适配检测器
      cv::resize(img, img, cv::Size(cod.image_width, cod.image_height));

      // 进行通用目标检测
      cod.detect(img, tgts);
      gimbalNoTrack();

      // 向控制台输出检测结果
      printf("Frame-[%d]\n", frame_id);
      // 打印当前检测的FPS
      printf("  FPS = %.2f\n", tgts.fps);
      // 打印当前相机的视场角（degree）
      printf("  FOV (fx, fy) = (%.2f, %.2f)\n", tgts.fov_x, tgts.fov_y);
      for (int i = 0; i < tgts.targets.size(); i++)
      {
        printf("Frame-[%d], Object-[%d]\n", frame_id, i);
        // 打印每个目标的中心位置，cx，cy的值域为[0, 1]
        printf("  Object Center (cx, cy) = (%.3f, %.3f)\n", tgts.targets[i].cx, tgts.targets[i].cy);
        // 打印每个目标的外接矩形框的宽度、高度，w，h的值域为(0, 1]
        printf("  Object Size (w, h) = (%.3f, %.3f)\n", tgts.targets[i].w, tgts.targets[i].h);
        // 打印每个目标的置信度
        printf("  Object Score = %.3f\n", tgts.targets[i].score);
        // 打印每个目标的类别，字符串类型
        printf("  Object Category = %s, Category ID = [%d]\n", tgts.targets[i].category.c_str(), tgts.targets[i].category_id);
        // 打印每个目标的视线角，跟相机视场相关
        printf("  Object Line-of-sight (ax, ay) = (%.3f, %.3f)\n", tgts.targets[i].los_ax, tgts.targets[i].los_ay);
        // 打印每个目标的3D位置（在相机坐标系下），跟目标实际长宽、相机参数相关
        printf("  Object Position = (x, y, z) = (%.3f, %.3f, %.3f)\n", tgts.targets[i].px, tgts.targets[i].py, tgts.targets[i].pz);
      }
    }
    else
    {
      // 缩放图像尺寸用于适配追踪器
      cv::resize(img, img, cv::Size(sot.image_width, sot.image_height));

      // 图像追踪
      sot.track(img, tgts);

      // 吊舱追踪
      gimbalTrack(tgts.targets[0].cx - 0.5f, tgts.targets[0].cy - 0.5f);

      // 向控制台输出追踪结果
      printf("Frame-[%d]\n", frame_id);
      // 打印当前检测的FPS
      printf("  FPS = %.2f\n", tgts.fps);
      // 打印当前相机的视场角（degree）
      printf("  FOV (fx, fy) = (%.2f, %.2f)\n", tgts.fov_x, tgts.fov_y);
      if (tgts.targets.size() > 0)
      {
        printf("Frame-[%d]\n", frame_id);
        // 打印 跟踪目标 的中心位置，cx，cy的值域为[0, 1]
        printf("  Tracking Center (cx, cy) = (%.3f, %.3f)\n", tgts.targets[0].cx, tgts.targets[0].cy);
        // 打印 跟踪目标 的外接矩形框的宽度、高度，w，h的值域为(0, 1]
        printf("  Tracking Size (w, h) = (%.3f, %.3f)\n", tgts.targets[0].w, tgts.targets[0].h);
        // 打印 跟踪目标 的视线角，跟相机视场相关
        printf("  Tracking Line-of-sight (ax, ay) = (%.3f, %.3f)\n", tgts.targets[0].los_ax, tgts.targets[0].los_ay);
      }
    }

    // 叠加目标框至原始图像
    sv::drawTargetsInFrame(img, tgts);
    // 显示检测结果img
    cv::imshow(RGB_WINDOW, img);
    cv::waitKey(10);

    lastTgts = tgts;
  }

  return 0;
}

void onMouse(int event, int x, int y, int, void *)
{
  // 左键点击 进入追踪模式
  if (event == cv::MouseEventTypes::EVENT_LBUTTONDOWN)
  {
    if (!isTarck && !isStartTarck)
    {
      clickX = x;
      clickY = y;
      isStartTarck = true;
    }
  }
  // 右键点击 退出追踪模式
  else if (event == cv::MouseEventTypes::EVENT_RBUTTONDOWN)
  {
    if (isTarck)
    {
      clickX = -1;
      clickY = -1;
      isTarck = false;
    }
  }
  // 中键点击 归中（仅非追踪模式下有效）
  else if (event == cv::MouseEventTypes::EVENT_MBUTTONDOWN)
  {
    if (!isTarck)
    {
      gimbal->setHome();
    }
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
