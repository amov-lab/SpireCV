#include <iostream>
#include <string>
// 包含SpireCV SDK头文件
#include <sv_world.h>

using namespace std;
using namespace cv;

//extract name
std::string GetImageFileName(const std::string& imagePath) {
  size_t lastSlash = imagePath.find_last_of("/\\");
  if (lastSlash == std::string::npos)
  {
    return imagePath;
  }
  else
  {
    std::string fileName = imagePath.substr(lastSlash + 1);
    size_t lastDot = fileName.find_last_of(".");
    if (lastDot != std::string::npos)
    {
      return fileName.substr(0, lastDot);
    }
    return fileName;
  }
}


int main(int argc, char *argv[])
{
  // 实例化 通用目标 检测器类
  sv::CommonObjectDetector cod;
  // 手动导入相机参数，如果使用Amov的G1等吊舱或相机，则可以忽略该步骤，将自动下载相机参数文件
  cod.loadCameraParams(sv::get_home() + "/SpireCV/calib_webcam_640x480.yaml");

  //load data
  string val_path = sv::get_home() + "/SpireCV/val2017/val2017"; 
  vector<string> val_image;
  glob(val_path, val_image, false);
  if (val_image.size() == 0)
  { 
    printf("val_image error!!!\n");
    exit(1);
  }

  //preds folder
  std::string folder = sv::get_home() + "/SpireCV/val2017/preds";
  int checkStatus = std::system(("if [ -d \"" + folder + "\" ]; then echo; fi").c_str());
  if(checkStatus == 0)
  {
    int removeStatus = std::system(("rm -rf \"" + folder + "\"").c_str());
    if(removeStatus != 0)
    {
      printf("remove older preds folder error!!!\n");
      exit(1);
    }
  } 

  int status = std::system(("mkdir \""+folder+"\"").c_str());
  if(status != 0)
  {
    printf("create preds folder error!!!\n");
    exit(1);
  }


  for (int i = 0; i < val_image.size(); i++)
  {
    //create pred file
    std::string val_image_name = GetImageFileName(val_image[i]);
    std::string filename = folder+"/"+ val_image_name + ".txt";
    std::ofstream file(filename);
    file.is_open();
    file<<std::fixed<<std::setprecision(6);

    // 实例化SpireCV的 单帧检测结果 接口类 TargetsInFrame
    sv::TargetsInFrame tgts(i);
    cv::Mat img = imread(val_image[i]);

    int rows = img.rows;
    int cols = img.cols;
   
    // 执行通用目标检测
    cod.detect(img, tgts);
    // 可视化检测结果，叠加到img上
    sv::drawTargetsInFrame(img, tgts);
    
    // reslusts
    for (int j = 0; j < tgts.targets.size(); j++)
    {
      sv::Box b;
      tgts.targets[j].getBox(b);
      file << tgts.targets[j].category_id << " " << (float)(b.x1+b.x2) / (2*cols) << " " << (float)(b.y1+b.y2) / (2*rows) << \
        " " << (float)(b.x2-b.x1) / cols << " " << (float)(b.y2-b.y1) / rows << " " << (float)tgts.targets[j].score << "\n";
    }
    file.close();
  }
  return 0;
}
