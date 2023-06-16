#include <iostream>
#include <string>
// 包含SpireCV SDK头文件
#include <sv_world.h>

using namespace std;

int main(int argc, char *argv[]) {
  // 打开摄像头
  sv::Camera cap;
  // cap.setWH(1280, 720);
  // cap.setFps(30);
  cap.open(sv::CameraType::WEBCAM, 0);  // CameraID 0

  // 实例化视频推流类sv::VideoStreamer
  sv::VideoStreamer streamer;
  // 初始化 推流分辨率(640, 480)，端口号8554，比特率1Mb
  streamer.setup(cv::Size(1280, 720), 8554, 1);
  // 实例化OpenCV的Mat类，用于内存单帧图像
  cv::Mat img;
  while (1)
  {
    // 读取一帧图像到img
    cap.read(img);
    cv::resize(img, img, cv::Size(1280, 720));
    // 将img推流到 地址：rtsp://ip:8554/live
    streamer.stream(img);
    
    // 显示检测结果img
    cv::imshow("img", img);
    cv::waitKey(10);
  }

  return 0;
}
