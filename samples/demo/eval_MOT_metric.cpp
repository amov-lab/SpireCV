#include <iostream>
#include <string>
#include <experimental/filesystem>
// 包含SpireCV SDK头文件
#include <sv_world.h>

using namespace std;
namespace fs = std::experimental::filesystem;

int main(int argc, char *argv[]) {
  // 实例化
  sv::CommonObjectDetector cod;
  // 手动导入相机参数，如果使用Amov的G1等吊舱或相机，则可以忽略该步骤，将自动下载相机参数文件
  cod.loadCameraParams(sv::get_home() + "/SpireCV/confs/calib_webcam_1280x720.yaml");
  cod.loadAlgorithmParams(sv::get_home() + "/SpireCV/confs/sv_algorithm_params.json");
  sv::MultipleObjectTracker mot;
  // 手动导入相机参数，如果使用Amov的G1等吊舱或相机，则可以忽略该步骤，将自动下载相机参数文件
  mot.loadCameraParams(sv::get_home() + "/SpireCV/confs/calib_webcam_1280x720.yaml");
  mot.loadAlgorithmParams(sv::get_home() + "/SpireCV/confs/sv_algorithm_params.json");
  mot.init(&cod);
  
  // 打开摄像头
  /*
  sv::Camera cap;
  cap.setWH(mot.image_width, mot.image_height);
  cap.setFps(30);
  cap.open(sv::CameraType::V4L2CAM, 0);  // CameraID 0
  */
  std::string mot17_folder_path = sv::get_home()+"/SpireCV/dataset/MOT17/train/";
  std::string pred_file_path = sv::get_home()+"/SpireCV/dataset/pred_mot17/data/";
  for (auto & seq_path : std::experimental::filesystem::directory_iterator(mot17_folder_path))
  { 
    // mkdir pred dirs and touch pred_files
    string pred_file = pred_file_path + seq_path.path().filename().string() + ".txt";
    fs::create_directories(pred_file_path);
    std::ofstream file(pred_file);
    // listdir seqence images
    string seq_image_paths = mot17_folder_path + seq_path.path().filename().string() + "/img1";
    // cout << seq_image_paths <<endl;
    std::vector<std::string> seq_image_file_path;
    cv::glob(seq_image_paths, seq_image_file_path);

    //eval MOT algorithms
    cv::Mat img;
    int frame_id = 0;
    while (frame_id < seq_image_file_path.size())
    {
      img = cv::imread(seq_image_file_path[frame_id]);
      // 实例化SpireCV的 单帧检测结果 接口类 TargetsInFrame
      sv::TargetsInFrame tgts(frame_id++);
      // 读取一帧图像到img
      //cap.read(img);
      //cv::resize(img, img, cv::Size(mot.image_width, mot.image_height));

      // 执行通用目标检测
      sv::TargetsInFrame person_tgts = mot.track(img, tgts);
      // 可视化检测结果，叠加到img上
      sv::drawTargetsInFrame(img, person_tgts);
      // printf("  Frame Size (width, height) = (%d, %d)\n", tgts.width, tgts.height);
      for (auto target : person_tgts.targets)
      {
          int center_x = int(target.cx * tgts.width);
          int center_y = int(target.cy * tgts.height);
          int width = int(target.w * tgts.width);
          int height = int(target.h * tgts.height);
          double conf = target.score;
          file << frame_id << ","<< target.tracked_id << "," << center_x - width / 2 << "," << center_y - height / 2 << "," << width << "," << height << "," << conf << "," << "-1,-1,-1" << endl;
          // file << frame_id << ","<< target.tracked_id << "," << center_x << "," << center_y << "," << width << "," << height << "," << conf << "," << "-1,-1,-1" << endl;
      }
      cv::imshow("img", img);
      cv::waitKey(10);
    }
    file.close();
  }
  return 0;
}