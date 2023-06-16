#include <iostream>
#include <string>
// 包含SpireCV SDK头文件
#include <sv_world.h>

using namespace std;

// 定义窗口名称
static const std::string RGB_WINDOW = "Image window";
// 框选到的矩形
cv::Rect rect_sel;
// 框选起始点
cv::Point pt_origin;
// 是否按下左键
bool b_clicked = false;
// 是否得到一个新的框选区域
bool b_renew_ROI = false;
// 是否开始跟踪
bool b_begin_TRACK = false;
// 实现框选逻辑的回调函数
void onMouse(int event, int x, int y, int, void*);

int main(int argc, char *argv[]) {
  // 定义一个新的窗口，可在上面进行框选操作
  cv::namedWindow(RGB_WINDOW);
  // 设置窗口操作回调函数，该函数实现整个框选逻辑
  cv::setMouseCallback(RGB_WINDOW, onMouse, 0);
  // 实例化 框选目标跟踪类
  sv::SingleObjectTracker sot;
  // 手动导入相机参数，如果使用Amov的G1等吊舱或相机，则可以忽略该步骤，将自动下载相机参数文件
  sot.loadCameraParams(sv::get_home() + "/SpireCV/calib_webcam_640x480.yaml");
  // sot.loadCameraParams(sv::get_home() + "/SpireCV/calib_webcam_1280x720.yaml");
  // sot.loadCameraParams(sv::get_home() + "/SpireCV/calib_webcam_1920x1080.yaml");
  
  // 打开摄像头
  sv::Camera cap;
  // cap.setWH(640, 480);
  // cap.setFps(30);
  cap.open(sv::CameraType::WEBCAM, 0);  // CameraID 0
  // cv::VideoCapture cap("/home/amov/SpireCV/test/tracking_1280x720.mp4");
  // 实例化OpenCV的Mat类，用于内存单帧图像
  cv::Mat img;
  int frame_id = 0;
  while (1)
  {
    // 实例化SpireCV的 单帧检测结果 接口类 TargetsInFrame
    sv::TargetsInFrame tgts(frame_id++);
    // 读取一帧图像到img
    cap.read(img);
    cv::resize(img, img, cv::Size(sot.image_width, sot.image_height));

    // 开始 单目标跟踪 逻辑
    // 是否有新的目标被手动框选
    if (b_renew_ROI)
    {
      // 拿新的框选区域 来 初始化跟踪器
      sot.init(img, rect_sel);
      // std::cout << rect_sel << std::endl;
      // 重置框选标志
      b_renew_ROI = false;
      // 开始跟踪
      b_begin_TRACK = true;
    }
    else if (b_begin_TRACK)
    {
      // 以前一帧的结果继续跟踪
      sot.track(img, tgts);

      // 可视化检测结果，叠加到img上
      sv::drawTargetsInFrame(img, tgts);

      // 控制台打印 单目标跟踪 结果
      printf("Frame-[%d]\n", frame_id);
      // 打印当前检测的FPS
      printf("  FPS = %.2f\n", tgts.fps);
      // 打印当前相机的视场角（degree）
      printf("  FOV (fx, fy) = (%.2f, %.2f)\n", tgts.fov_x, tgts.fov_y);
      // 打印当前输入图像的像素宽度和高度
      printf("  Frame Size (width, height) = (%d, %d)\n", tgts.width, tgts.height);
      if (tgts.targets.size() > 0)
      {
        printf("Frame-[%d]\n", frame_id);
        // 打印 跟踪目标 的中心位置，cx，cy的值域为[0, 1]，以及cx，cy的像素值
        printf("  Tracking Center (cx, cy) = (%.3f, %.3f), in Pixels = ((%d, %d))\n",
          tgts.targets[0].cx, tgts.targets[0].cy,
          int(tgts.targets[0].cx * tgts.width),
          int(tgts.targets[0].cy * tgts.height));
        // 打印 跟踪目标 的外接矩形框的宽度、高度，w，h的值域为(0, 1]，以及w，h的像素值
        printf("  Tracking Size (w, h) = (%.3f, %.3f), in Pixels = ((%d, %d))\n",
          tgts.targets[0].w, tgts.targets[0].h,
          int(tgts.targets[0].w * tgts.width),
          int(tgts.targets[0].h * tgts.height));
        // 打印 跟踪目标 的视线角，跟相机视场相关
        printf("  Tracking Line-of-sight (ax, ay) = (%.3f, %.3f)\n", tgts.targets[0].los_ax, tgts.targets[0].los_ay);
      }

    }
    
    // 显示检测结果img
    cv::imshow(RGB_WINDOW, img);
    cv::waitKey(10);
  }

  return 0;
}

void onMouse(int event, int x, int y, int, void*)
{
  if (b_clicked)
  {
    // 更新框选区域坐标
    rect_sel.x = MIN(pt_origin.x, x);        
    rect_sel.y = MIN(pt_origin.y, y);
    rect_sel.width = abs(x - pt_origin.x);   
    rect_sel.height = abs(y - pt_origin.y);
  }
  // 左键按下
  if (event == cv::EVENT_LBUTTONDOWN)
  {
    b_begin_TRACK = false;  
    b_clicked = true; 
    pt_origin = cv::Point(x, y);       
    rect_sel = cv::Rect(x, y, 0, 0);  
  }
  // 左键松开
  else if (event == cv::EVENT_LBUTTONUP)
  {
    // 框选区域需要大于8x8像素
    if (rect_sel.width * rect_sel.height < 64)
    {
      ;
    }
    else
    {
      b_clicked = false;
      b_renew_ROI = true;    
    } 
  }
}
