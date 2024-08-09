#include <iostream>
#include <string>
// 包含SpireCV SDK头文件
#include <sv_world.h>

using namespace std;

int main(int argc, char *argv[])
{
  // 实例化 通用目标 检测器类
  sv::MonocularDepthEstimation mde;
  // 手动导入相机参数，如果使用Amov的G1等吊舱或相机，则可以忽略该步骤，将自动下载相机参数文件
  mde.loadCameraParams(sv::get_home() + "/SpireCV/confs/calib_webcam_640x480.yaml");
  mde.loadAlgorithmParams(sv::get_home() + "/SpireCV/confs/sv_algorithm_params.json");

  // 打开摄像头
  sv::Camera cap;
  cap.setWH(mde.image_width, mde.image_height);
  cap.setFps(30);
  cap.open(sv::CameraType::V4L2CAM, 0); // CameraID 0
  // 实例化OpenCV的Mat类，用于内存单帧图像
  cv::Mat img;
  int frame_id = 0;
  while (1)
  {
    // 实例化SpireCV的 单帧检测结果 接口类 TargetsInFrame
    sv::TargetsInFrame tgts(frame_id++);
    // 读取一帧图像到img
    cap.read(img);
    cv::resize(img, img, cv::Size(mde.image_width, mde.image_height));
    cv::imshow("img", img);

    // 执行MonocularDepthEstimation
    mde.predict(img, tgts);

    // 控制台打印通用目标检测结果
    printf("Frame-[%d]\n", frame_id);
    // 打印当前检测的FPS
    printf("  FPS = %.2f\n", tgts.fps);
    // 打印当前相机的视场角（degree）
    printf("  FOV (fx, fy) = (%.2f, %.2f)\n", tgts.fov_x, tgts.fov_y);
    // 打印当前输入图像的像素宽度和高度
    printf("  Frame Size (width, height) = (%d, %d)\n", tgts.width, tgts.height);

    // 归一化深度图到0-1范围  
    double minVal, maxVal;  
    cv::minMaxLoc(tgts.depth_data, &minVal, &maxVal); 

    // 深度图可视化
    cv::Mat depth_mat;
    cv::normalize(tgts.depth_data, depth_mat, 0, 255, cv::NORM_MINMAX, CV_8U);

     // 根据深度数据创建色域图
    cv::Mat colormap;
    cv::applyColorMap(depth_mat, colormap, cv::COLORMAP_JET);
    cv::resize(colormap, colormap, cv::Size(mde.image_width, mde.image_height));
    
    cv::putText(colormap, cv::format("maxVal: %.1f m", maxVal), cv::Point(200, 20),
                0, 0.6, cv::Scalar(255, 99, 71), 2, cv::LINE_AA);
    cv::putText(colormap, cv::format("minVal: %.1f m", minVal), cv::Point(20, 20),
                0, 0.6, cv::Scalar(255, 99, 71), 2, cv::LINE_AA);     

    // 合并当前帧和深度图
    cv::Mat showImage;
    cv::hconcat(img, colormap, showImage);

    // 显示检测结果img
    cv::imshow("img", showImage);
    cv::waitKey(1);
  }

  return 0;
}
