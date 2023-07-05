#ifndef __SV_VIDEOIO__
#define __SV_VIDEOIO__

#include <string>
#include <vector>
#include <opencv2/opencv.hpp>
#include <fstream>
#include <queue>
#include <stack>
#include <thread>

#include <arpa/inet.h>
#include <netinet/in.h>  // for sockaddr_in

#define SV_RAD2DEG 57.2957795
// #define X86_PLATFORM
// #define JETSON_PLATFORM


namespace sv {

//! The rectangle bounding-box of an object.
class Box
{
public:
  Box();

  int x1;
  int y1;
  int x2;
  int y2;

  //! Set the parameters of the bounding-box by XYXY-format.
  /*!
    \param x1_: The x-axis pixel coordinates of the top-left point.
    \param y1_: The y-axis pixel coordinates of the top-left point.
    \param x2_: The x-axis pixel coordinates of the bottom-right point.
    \param y2_: The y-axis pixel coordinates of the bottom-right point.
  */
  void setXYXY(int x1_, int y1_, int x2_, int y2_);
  //! Set the parameters of the bounding-box by XYWH-format.
  /*!
    \param x1_: The x-axis pixel coordinates of the top-left point.
    \param y1_: The y-axis pixel coordinates of the top-left point.
    \param w_: The width of the bounding rectangle.
    \param h_: The height of the bounding rectangle.
  */
  void setXYWH(int x_, int y_, int w_, int h_);
};


//! Description class for a single target detection result.
/*!
  Support multiple description methods,
  such as bounding box, segmentation, ellipse, three-dimensional position, etc.
*/
class Target
{
public:
  Target();

  //! X coordinate of object center point, [0, 1], (Required)
  double cx;
  //! Y coordinate of object center point, [0, 1], (Required)
  double cy;
  //! Object-width / image-width, (0, 1]
  double w;
  //! Object-height / image-heigth, (0, 1]
  double h;

  //! Objectness, Confidence, (0, 1]
  double score;
  //! Category of target.
  std::string category;
  //! Category ID of target.
  int category_id;
  //! The same target in different frames shares a unique ID.
  int tracked_id;

  //! X coordinate of object position in Camera-Frame (unit: meter).
  double px;
  //! Y coordinate of object position in Camera-Frame (unit: meter).
  double py;
  //! Z coordinate of object position in Camera-Frame (unit: meter).
  double pz;

  //! Line of sight (LOS) angle on X-axis (unit: degree).
  double los_ax;
  //! Line of sight (LOS) angle on Y-axis (unit: degree).
  double los_ay;
  //! The angle of the target in the image coordinate system,  (unit: degree) [-180, 180].
  double yaw_a;

  //! Whether the height&width of the target can be obtained.
  bool has_hw;
  //! Whether the category of the target can be obtained.
  bool has_category;
  //! Whether the tracking-ID of the target can be obtained.
  bool has_tid;
  //! Whether the 3D-position of the target can be obtained.
  bool has_position;
  //! Whether the LOS-angle of the target can be obtained.
  bool has_los;
  //! Whether the segmentation of the target can be obtained.
  bool has_seg;
  //! Whether the bounding-box of the target can be obtained.
  bool has_box;
  //! Whether the ellipse-parameters of the target can be obtained.
  bool has_ell;
  //! Whether the aruco-parameters of the target can be obtained.
  bool has_aruco;
  //! Whether the direction of the target can be obtained.
  bool has_yaw;

  void setCategory(std::string cate_, int cate_id_);
  void setLOS(double cx_, double cy_, cv::Mat camera_matrix_, int img_w_, int img_h_);
  void setTrackID(int id_);
  void setPosition(double x_, double y_, double z_);
  void setBox(int x1_, int y1_, int x2_, int y2_, int img_w_, int img_h_);
  void setAruco(int id_, std::vector<cv::Point2f> corners_, cv::Vec3d rvecs_, cv::Vec3d tvecs_, int img_w_, int img_h_, cv::Mat camera_matrix_);
  void setEllipse(double xc_, double yc_, double a_, double b_, double rad_, double score_, int img_w_, int img_h_, cv::Mat camera_matrix_, double radius_in_meter_);
  void setYaw(double vec_x_, double vec_y);
  void setMask(cv::Mat mask_);
  cv::Mat getMask();
  
  bool getBox(Box& b);
  bool getAruco(int& id, std::vector<cv::Point2f> &corners);
  bool getEllipse(double& xc_, double& yc_, double& a_, double& b_, double& rad_);
  std::string getJsonStr();

private:
  //! segmentation [[x1,y1, x2,y2, x3,y3,...],...]
  /*!
    SEG variables: (_s_) segmentation, segmentation_size_h, segmentation_size_w, segmentation_counts, area
  */
  std::vector<std::vector<double> > _s_segmentation;
  int _s_segmentation_size_h;
  int _s_segmentation_size_w;
  std::string _s_segmentation_counts;
  cv::Mat _mask;
  double _s_area;
  //! bounding box [x, y, w, h]
  /*!
    BOX variables: (_b_) box
  */
  Box _b_box;  // x,y,w,h
  //! ellipse x-axis center
  /*!
    ELL variables: (_e_) xc, yc, a, b, rad
  */
  double _e_xc;
  double _e_yc;
  double _e_a;
  double _e_b;
  double _e_rad;
  //! Aruco Marker ID
  /*!
    ARUCO variables: (_a_) id, corners, rvecs, tvecs
  */
  int _a_id;
  std::vector<cv::Point2f> _a_corners;
  cv::Vec3d _a_rvecs;
  cv::Vec3d _a_tvecs;
};


enum class MissionType {NONE, COMMON_DET, TRACKING, ARUCO_DET, LANDMARK_DET, ELLIPSE_DET};

//! This class describes all objects in a single frame image.
/*!
  1. Contains multiple Target instances.
  2. Describes the ID of the current frame, image width and height, current field of view, etc.
  3. Describes the processed image sub-regions and supports local region detection.
*/
class TargetsInFrame
{
public:
  TargetsInFrame(int frame_id_);

  //! Frame number.
  int frame_id;
  //! Frame/image height.
  int height;
  //! Frame/image width.
  int width;

  //! Detection frame per second (FPS).
  double fps;
  //! The x-axis field of view (FOV) of the current camera.
  double fov_x;
  //! The y-axis field of view (FOV) of the current camera.
  double fov_y;

  //! 吊舱俯仰角
  double pod_patch;
  //! 吊舱滚转角
  double pod_roll;
  //! 吊舱航向角，东向为0，东北天为正，范围[-180,180]
  double pod_yaw;

  //! 当前经度
  double longitude;
  //! 当前纬度
  double latitude;
  //! 当前飞行高度
  double altitude;

  //! 飞行速度，x轴，东北天坐标系
  double uav_vx;
  //! 飞行速度，y轴，东北天坐标系
  double uav_vy;
  //! 飞行速度，z轴，东北天坐标系
  double uav_vz;
  //! 当前光照强度，Lux
  double illumination;

  //! Whether the detection FPS can be obtained.
  bool has_fps;
  //! Whether the FOV can be obtained.
  bool has_fov;
  //! Whether the processed image sub-region can be obtained.
  bool has_roi;

  bool has_pod_info;
  bool has_uav_pos;
  bool has_uav_vel;
  bool has_ill;

  MissionType type;

  //! The processed image sub-region, if size>0, it means no full image detection.
  std::vector<Box> rois;
  //! Detected Target Instances.
  std::vector<Target> targets;
  std::string date_captured;
  
  void setTimeNow();
  void setFPS(double fps_);
  void setFOV(double fov_x_, double fov_y_);
  void setSize(int width_, int height_);
  std::string getJsonStr();
};


class UDPServer {
public:
  UDPServer(std::string dest_ip="127.0.0.1", int port=20166);
  ~UDPServer();

  void send(const TargetsInFrame& tgts_);
private:
  struct sockaddr_in _servaddr;
  int _sockfd;
};


class VideoWriterBase {
public:
  VideoWriterBase();
  ~VideoWriterBase();
  
  void setup(std::string file_path, cv::Size size, double fps=25.0, bool with_targets=false);
  void write(cv::Mat image, TargetsInFrame tgts=TargetsInFrame(0));
  void release();
  
  cv::Size getSize();
  double getFps();
  std::string getFilePath();
  bool isRunning();
protected:
  virtual bool setupImpl(std::string file_name_);
  virtual bool isOpenedImpl();
  virtual void writeImpl(cv::Mat img_);
  virtual void releaseImpl();
  void _init();
  void _run();
  
  bool _is_running;
  cv::Size _image_size;
  double _fps;
  bool _with_targets;  
  int _fid;
  int _fcnt;
  
  std::thread _tt;
  // cv::VideoWriter _writer;
  std::ofstream _targets_ofs;
  std::string _file_path;

  std::queue<cv::Mat> _image_to_write;
  std::queue<TargetsInFrame> _tgts_to_write;
};


class VideoStreamerBase {
public:
  VideoStreamerBase();
  ~VideoStreamerBase();

  void setup(cv::Size size, int port=8554, int bitrate=2, std::string url="/live");  // 2M
  void stream(cv::Mat image);
  void release();
  
  cv::Size getSize();
  int getPort();
  std::string getUrl();
  int getBitrate();
  bool isRunning();
protected:
  virtual bool setupImpl();
  virtual bool isOpenedImpl();
  virtual void writeImpl(cv::Mat image);
  virtual void releaseImpl();
  void _run();

  bool _is_running;
  cv::Size _stream_size;
  int _port;
  std::string _url;
  int _bitrate;
  std::thread _tt;
  std::stack<cv::Mat> _image_to_stream;
};


enum class CameraType {NONE, WEBCAM, G1, Q10, MIPI};

class CameraBase {
public:
  CameraBase(CameraType type=CameraType::NONE, int id=0);
  ~CameraBase();
  void open(CameraType type=CameraType::WEBCAM, int id=0);
  bool read(cv::Mat& image);
  void release();
  
  int getW();
  int getH();
  int getFps();
  std::string getIp();
  int getPort();
  double getBrightness();
  double getContrast();
  double getSaturation();
  double getHue();
  double getExposure();
  bool isRunning();
  void setWH(int width, int height);
  void setFps(int fps);
  void setIp(std::string ip);
  void setPort(int port);
  void setBrightness(double brightness);
  void setContrast(double contrast);
  void setSaturation(double saturation);
  void setHue(double hue);
  void setExposure(double exposure);
protected:
  virtual void openImpl();
  void _run();

  bool _is_running;
  bool _is_updated;
  std::thread _tt;
  cv::VideoCapture _cap;
  cv::Mat _frame;
  CameraType _type;
  int _camera_id;
  
  int _width;
  int _height;
  int _fps;
  std::string _ip;
  int _port;
  double _brightness;
  double _contrast;
  double _saturation;
  double _hue;
  double _exposure;
};


void drawTargetsInFrame(
  cv::Mat& img_,
  const TargetsInFrame& tgts_,
  bool with_all=true,
  bool with_category=false,
  bool with_tid=false,
  bool with_seg=false,
  bool with_box=false,
  bool with_ell=false,
  bool with_aruco=false,
  bool with_yaw=false
);
std::string get_home();
bool is_file_exist(std::string& fn);
void list_dir(std::string dir, std::vector<std::string>& files, std::string suffixs="", bool r=false);


}
#endif
