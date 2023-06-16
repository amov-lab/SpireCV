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

// 是否得到一个新的框选区域
bool b_renew_ROI = false;
// 是否开始跟踪
bool b_begin_TRACK = false;
// 实现框选逻辑的回调函数
void onMouse(int event, int x, int y, int, void*);


struct node {
  double x,y;
};
node p1,p2,p3,p4;
node p;
double getCross(node p1, node p2, node p) {
  return (p2.x-p1.x)*(p.y-p1.y)-(p.x-p1.x)*(p2.y-p1.y);
}
bool b_clicked =false;
bool detect_tracking =true;


int main(int argc, char *argv[]) {
  // 定义一个新的窗口，可在上面进行框选操作
  cv::namedWindow(RGB_WINDOW);
  // 设置窗口操作回调函数，该函数实现整个框选逻辑
  cv::setMouseCallback(RGB_WINDOW, onMouse, 0);
  // 实例化 框选目标跟踪类
  sv::SingleObjectTracker sot;
  // 手动导入相机参数，如果使用Amov的G1等吊舱或相机，则可以忽略该步骤，将自动下载相机参数文件
  sot.loadCameraParams(sv::get_home() + "/SpireCV/calib_webcam_640x480.yaml");


  sv::CommonObjectDetector cod;
  cod.loadCameraParams(sv::get_home() + "/SpireCV/calib_webcam_640x480.yaml");


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
    if (detect_tracking == true) {
      // 实例化SpireCV的 单帧检测结果 接口类 TargetsInFrame
      sv::TargetsInFrame tgts(frame_id++);
      // 读取一帧图像到img
      cap.read(img);
      cv::resize(img, img, cv::Size(cod.image_width, cod.image_height));

      // 执行通用目标检测
      cod.detect(img, tgts);

      // 可视化检测结果，叠加到img上
      sv::drawTargetsInFrame(img, tgts);

      // 控制台打印通用目标检测结果
      printf("Frame-[%d]\n", frame_id);
      // 打印当前检测的FPS
      printf("  FPS = %.2f\n", tgts.fps);
      // 打印当前相机的视场角（degree）
      printf("  FOV (fx, fy) = (%.2f, %.2f)\n", tgts.fov_x, tgts.fov_y);
      for (int i=0; i<tgts.targets.size(); i++)
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
        p1.x = tgts.targets[i].cx * tgts.width  - tgts.targets[i].w * tgts.width  / 2;
        p1.y = tgts.targets[i].cy * tgts.height - tgts.targets[i].h * tgts.height / 2;
        p2.x = tgts.targets[i].cx * tgts.width  + tgts.targets[i].w * tgts.width  / 2;
        p2.y = tgts.targets[i].cy * tgts.height - tgts.targets[i].h * tgts.height / 2;
        p4.x = tgts.targets[i].cx * tgts.width  - tgts.targets[i].w * tgts.width  / 2;
        p4.y = tgts.targets[i].cy * tgts.height + tgts.targets[i].h * tgts.height / 2;
        p3.x = tgts.targets[i].cx * tgts.width  + tgts.targets[i].w * tgts.width  / 2;
        p3.y = tgts.targets[i].cy * tgts.height + tgts.targets[i].h * tgts.height / 2;
        p.x = pt_origin.x;
        p.y = pt_origin.y;
        std::cout << "p.x " << p.x << "\t" << "p.y " << p.y << std::endl;
        if (getCross(p1, p2, p) * getCross(p3, p4, p) >= 0 && getCross(p2, p3, p) * getCross(p4, p1, p) >= 0) {
          b_begin_TRACK = false;  
          detect_tracking = false;
          // pt_origin = cv::Point(nor_x, nor_p_y); 
          // std::cout << "pt_origin  " <<nor_x<<"/t"<<nor_p_y<< std::endl;      
          rect_sel = cv::Rect(p1.x, p1.y, tgts.targets[i].w * tgts.width, tgts.targets[i].h * tgts.height);  
          // std::cout << rect_sel << std::endl;
          b_renew_ROI = true;
          frame_id = 0;
          printf("rect_sel Yes\n");
        }
        else {
          printf("rect_sel No\n");
        }
      }
    }
    else {
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
    }//end of tracking     
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
    pt_origin.x = 0;
    pt_origin.y = 0;
  }
  // 左键按下
  if (event == cv::EVENT_LBUTTONDOWN)
  {
    detect_tracking = true;
    pt_origin = cv::Point(x, y);  
  }

  else if (event == cv::EVENT_RBUTTONDOWN)
  {
    detect_tracking = true;
    b_renew_ROI = false;
    b_begin_TRACK = false;
    b_clicked = true;
  }
}
