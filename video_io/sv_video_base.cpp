#include "sv_video_base.h"
#include <opencv2/aruco.hpp>
#include "ellipse_detector.h"
#include "sv_util.h"
#include "sv_crclib.h"

#define SV_MAX_FRAMES 52000
typedef unsigned char byte;


namespace sv {


cv::Ptr<cv::aruco::Dictionary> _g_dict = nullptr;


std::string get_home()
{
  return _get_home();
}
bool is_file_exist(std::string& fn)
{
  return _is_file_exist(fn);
}
void list_dir(std::string dir, std::vector<std::string>& files, std::string suffixs, bool r)
{
  yaed::_list_dir(dir, files, suffixs, r);
}

cv::Mat& _attach_aruco(int id, cv::Mat& img)
{
  cv::Mat marker_img;
  std::vector<cv::Mat> ch(3);
  cv::aruco::Dictionary dict = cv::aruco::getPredefinedDictionary(cv::aruco::DICT_5X5_1000);
  ch[0] = cv::Mat::zeros(22, 22, CV_8UC1);
  ch[1] = cv::Mat::zeros(22, 22, CV_8UC1);
  ch[2] = cv::Mat::zeros(22, 22, CV_8UC1);

  ch[0].setTo(cv::Scalar(255));
  ch[1].setTo(cv::Scalar(255));
  ch[2].setTo(cv::Scalar(255));
  cv::Rect inner_roi = cv::Rect(4, 4, 14, 14);
  cv::Rect full_roi = cv::Rect(img.cols - 22, img.rows - 22, 22, 22);

  int id_k = id % 1000;

  // dict.drawMarker(id_k, 14, marker_img, 1);
  cv::aruco::generateImageMarker(dict, id_k, 14, marker_img, 1);
  marker_img.copyTo(ch[0](inner_roi));
  // dict.drawMarker(id_k, 14, marker_img, 1);
  cv::aruco::generateImageMarker(dict, id_k, 14, marker_img, 1);
  marker_img.copyTo(ch[1](inner_roi));
  // dict.drawMarker(id_k, 14, marker_img, 1);
  cv::aruco::generateImageMarker(dict, id_k, 14, marker_img, 1);
  marker_img.copyTo(ch[2](inner_roi));

  cv::merge(ch, marker_img);
  marker_img.copyTo(img(full_roi));
  return img;
}

int _parse_aruco(cv::Mat& img)
{
  int id;
  cv::Mat marker_img;
  std::vector<cv::Mat> ch(3);
  if (_g_dict == nullptr)
  {
    _g_dict = new cv::aruco::Dictionary;
    *_g_dict = cv::aruco::getPredefinedDictionary(cv::aruco::DICT_5X5_1000);
  }
  cv::Rect full_roi = cv::Rect(img.cols - 22, img.rows - 22, 22, 22);
  img(full_roi).copyTo(marker_img);
  cv::split(marker_img, ch);

  std::vector<int> id_i;
  std::vector<int> id_k;
  std::vector<int> id_m;
  std::vector<std::vector<cv::Point2f> > marker_corners;
  cv::aruco::detectMarkers(ch[0], _g_dict, marker_corners, id_i);
  cv::aruco::detectMarkers(ch[1], _g_dict, marker_corners, id_k);
  cv::aruco::detectMarkers(ch[2], _g_dict, marker_corners, id_m);
  if (id_i.size() > 0 || id_k.size() > 0 || id_m.size() > 0)
  {
    if (id_i.size() > 0)
      id = id_i[0];
    else if (id_k.size() > 0)
      id = id_k[0];
    else if (id_m.size() > 0)
      id = id_m[0];
  }
  else
  {
    // std::cout << "error ch0 & ch1" << std::endl;
    id = -1;
  }

  return id;
}


Target::Target()
{
  this->has_hw = false;
  this->has_tid = false;
  this->has_position = false;
  this->has_los = false;
  this->has_seg = false;
  this->has_box = false;
  this->has_ell = false;
  this->has_aruco = false;
  this->has_yaw = false;
}


UDPServer::UDPServer(std::string dest_ip, int port)
{
  this->_sockfd = socket(AF_INET, SOCK_DGRAM, 0);
  bzero(&this->_servaddr, sizeof(this->_servaddr));
  this->_servaddr.sin_family = AF_INET;
  inet_pton(AF_INET, dest_ip.c_str(), &this->_servaddr.sin_addr);
  this->_servaddr.sin_port = htons(port);
}

UDPServer::~UDPServer()
{

}

void _floatTobytes(float data, byte bytes[])
{
  int i;
  size_t length = sizeof(float);
  byte *pdata = (byte*)&data;
  for (i = 0; i < length; i++)
  {
    bytes[i] = *pdata++;
  }
}
void _intTobytes(int data, byte bytes[])
{
  int i;
  size_t length = sizeof(int);
  byte *pdata = (byte*)&data;
  for (i = 0; i < length; i++)
  {
    bytes[i] = *pdata++;
  }
}
void _uint32Tobytes(uint32_t data, byte bytes[])
{
  int i;
  size_t length = sizeof(uint32_t);
  byte *pdata = (byte*)&data;
  for (i = 0; i < length; i++)
  {
    bytes[i] = *pdata++;
  }
}
void _shortTobytes(unsigned short data, byte bytes[])
{
  int i;
  size_t length = sizeof(unsigned short);
  byte *pdata = (byte*)&data;
  for (i = 0; i < length; i++)
  {
    bytes[i] = *pdata++;
  }
}

void UDPServer::send(const TargetsInFrame& tgts_)
{
  byte upd_msg[1024*6];  // max to 100 objects

  upd_msg[0] = 0xFA;
  upd_msg[1] = 0xFC;

  upd_msg[4] = (byte) tgts_.type;
  upd_msg[5] = 0xFF;
  upd_msg[6] = 0xFF;
  upd_msg[7] = 0xFF;
  upd_msg[8] = 0xFF;

  std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
  std::chrono::system_clock::duration tp = now.time_since_epoch();
  tp -= std::chrono::duration_cast<std::chrono::seconds>(tp);
  unsigned short milliseconds = static_cast<unsigned short>(tp / std::chrono::milliseconds(1));
  std::time_t tt = std::chrono::system_clock::to_time_t(now);
  tm t = *std::localtime(&tt);

  _shortTobytes((unsigned short) (t.tm_year + 1900), &upd_msg[9]);  // year
  upd_msg[11] = (byte) (t.tm_mon + 1);  // month
  upd_msg[12] = (byte) t.tm_mday;  // day
  upd_msg[13] = (byte) t.tm_hour;  // hour
  upd_msg[14] = (byte) t.tm_min;  // min
  upd_msg[15] = (byte) t.tm_sec;  // sec
  _shortTobytes(milliseconds, &upd_msg[16]);
  int index_d1 = 18;
  upd_msg[index_d1] = 0x00;
  int index_d2 = 19;
  upd_msg[index_d2] = 0x00;
  int index_d3 = 20;
  upd_msg[index_d3] = 0x00;
  int index_d4 = 21;
  upd_msg[index_d4] = 0x00;

  int max_objs = 100;
  if (tgts_.targets.size() < 100)  max_objs = (int) tgts_.targets.size();

  int mp = 22;
  _intTobytes(tgts_.frame_id, &upd_msg[mp]);
  upd_msg[index_d4] = upd_msg[index_d4] | 0x01;
  mp += 4;
  _intTobytes(tgts_.width, &upd_msg[mp]);
  upd_msg[index_d4] = upd_msg[index_d4] | 0x02;
  mp += 4;
  _intTobytes(tgts_.height, &upd_msg[mp]);
  upd_msg[index_d4] = upd_msg[index_d4] | 0x04;
  mp += 4;
  _intTobytes(max_objs, &upd_msg[mp]);
  upd_msg[index_d4] = upd_msg[index_d4] | 0x08;
  if (tgts_.has_fps)
  {
    mp += 4;
    _floatTobytes((float) tgts_.fps, &upd_msg[mp]);
    upd_msg[index_d4] = upd_msg[index_d4] | 0x10;
  }
  if (tgts_.has_fov)
  {
    mp += 4;
    _floatTobytes((float) tgts_.fov_x, &upd_msg[mp]);
    upd_msg[index_d4] = upd_msg[index_d4] | 0x20;
    mp += 4;
    _floatTobytes((float) tgts_.fov_y, &upd_msg[mp]);
    upd_msg[index_d4] = upd_msg[index_d4] | 0x40;
  }
  if (tgts_.has_pod_info)
  {
    mp += 4;
    _floatTobytes((float) tgts_.pod_patch, &upd_msg[mp]);
    upd_msg[index_d4] = upd_msg[index_d4] | 0x80;
    mp += 4;
    _floatTobytes((float) tgts_.pod_roll, &upd_msg[mp]);
    upd_msg[index_d3] = upd_msg[index_d3] | 0x01;
    mp += 4;
    _floatTobytes((float) tgts_.pod_yaw, &upd_msg[mp]);
    upd_msg[index_d3] = upd_msg[index_d3] | 0x02;
  }
  if (tgts_.has_uav_pos)
  {
    mp += 4;
    _floatTobytes((float) tgts_.longitude, &upd_msg[mp]);
    upd_msg[index_d3] = upd_msg[index_d3] | 0x04;
    mp += 4;
    _floatTobytes((float) tgts_.latitude, &upd_msg[mp]);
    upd_msg[index_d3] = upd_msg[index_d3] | 0x08;
    mp += 4;
    _floatTobytes((float) tgts_.altitude, &upd_msg[mp]);
    upd_msg[index_d3] = upd_msg[index_d3] | 0x10;
  }
  if (tgts_.has_uav_vel)
  {
    mp += 4;
    _floatTobytes((float) tgts_.uav_vx, &upd_msg[mp]);
    upd_msg[index_d3] = upd_msg[index_d3] | 0x20;
    mp += 4;
    _floatTobytes((float) tgts_.uav_vy, &upd_msg[mp]);
    upd_msg[index_d3] = upd_msg[index_d3] | 0x40;
    mp += 4;
    _floatTobytes((float) tgts_.uav_vz, &upd_msg[mp]);
    upd_msg[index_d3] = upd_msg[index_d3] | 0x80;
  }
  if (tgts_.has_ill)
  {
    mp += 4;
    _floatTobytes((float) tgts_.illumination, &upd_msg[mp]);
    upd_msg[index_d2] = upd_msg[index_d2] | 0x01;
  }
  mp += 4;

  for (int n=0; n<max_objs; n++)
  {
    int index_f1 = mp;
    upd_msg[mp] = 0x00;
    mp++;
    int index_f2 = mp;
    upd_msg[mp] = 0x00;
    mp++;
    int index_f3 = mp;
    upd_msg[mp] = 0x00;
    mp++;
    int index_f4 = mp;
    upd_msg[mp] = 0x00;

    mp++;
    _floatTobytes((float) tgts_.targets[n].cx, &upd_msg[mp]);
    upd_msg[index_f4] = upd_msg[index_f4] | 0x01;
    mp += 4;
    _floatTobytes((float) tgts_.targets[n].cy, &upd_msg[mp]);
    upd_msg[index_f4] = upd_msg[index_f4] | 0x02;
    if (tgts_.targets[n].has_hw)
    {
      mp += 4;
      _floatTobytes((float) tgts_.targets[n].w, &upd_msg[mp]);
      upd_msg[index_f4] = upd_msg[index_f4] | 0x04;
      mp += 4;
      _floatTobytes((float) tgts_.targets[n].h, &upd_msg[mp]);
      upd_msg[index_f4] = upd_msg[index_f4] | 0x08;
    }
    mp += 4;
    _floatTobytes((float) tgts_.targets[n].score, &upd_msg[mp]);
    upd_msg[index_f4] = upd_msg[index_f4] | 0x10;
    if (tgts_.targets[n].has_category)
    {
      mp += 4;
      _intTobytes(tgts_.targets[n].category_id, &upd_msg[mp]);
      upd_msg[index_f4] = upd_msg[index_f4] | 0x20;
    }
    if (tgts_.targets[n].has_tid)
    {
      mp += 4;
      _intTobytes(tgts_.targets[n].tracked_id, &upd_msg[mp]);
      upd_msg[index_f4] = upd_msg[index_f4] | 0x40;
    }
    if (tgts_.targets[n].has_position)
    {
      mp += 4;
      _floatTobytes((float) tgts_.targets[n].px, &upd_msg[mp]);
      upd_msg[index_f4] = upd_msg[index_f4] | 0x80;
      mp += 4;
      _floatTobytes((float) tgts_.targets[n].py, &upd_msg[mp]);
      upd_msg[index_f3] = upd_msg[index_f3] | 0x01;
      mp += 4;
      _floatTobytes((float) tgts_.targets[n].pz, &upd_msg[mp]);
      upd_msg[index_f3] = upd_msg[index_f3] | 0x02;
    }
    if (tgts_.targets[n].has_los)
    {
      mp += 4;
      _floatTobytes((float) tgts_.targets[n].los_ax, &upd_msg[mp]);
      upd_msg[index_f3] = upd_msg[index_f3] | 0x04;
      mp += 4;
      _floatTobytes((float) tgts_.targets[n].los_ay, &upd_msg[mp]);
      upd_msg[index_f3] = upd_msg[index_f3] | 0x08;
    }
    if (tgts_.targets[n].has_yaw)
    {
      mp += 4;
      _floatTobytes((float) tgts_.targets[n].yaw_a, &upd_msg[mp]);
      upd_msg[index_f3] = upd_msg[index_f3] | 0x10;
    }
    mp += 4;
  }
  _shortTobytes((unsigned short) (mp + 4 - 2), &upd_msg[2]); // Length

  uint32_t crc = crc32(&upd_msg[2], (uint16_t) mp);
  _uint32Tobytes(crc, &upd_msg[mp]);
  mp += 4;

  upd_msg[mp] = 0xFB;
  mp++;
  upd_msg[mp] = 0xFD;
  mp++;

  // std::cout << mp << std::endl;

  int r = sendto(this->_sockfd, upd_msg, mp, 0, (struct sockaddr *)&this->_servaddr, sizeof(this->_servaddr));
}


void drawTargetsInFrame(
  cv::Mat& img_,
  const TargetsInFrame& tgts_,
  bool with_all,
  bool with_category,
  bool with_tid,
  bool with_seg,
  bool with_box,
  bool with_ell,
  bool with_aruco,
  bool with_yaw
)
{
  if (tgts_.rois.size() > 0)
  {
    cv::Mat image_ret;
    cv::addWeighted(img_, 0.5, cv::Mat::zeros(cv::Size(img_.cols, img_.rows), CV_8UC3), 0, 0, image_ret);
    cv::Rect roi = cv::Rect(tgts_.rois[0].x1, tgts_.rois[0].y1, tgts_.rois[0].x2 - tgts_.rois[0].x1, tgts_.rois[0].y2 - tgts_.rois[0].y1);
    img_(roi).copyTo(image_ret(roi));
    image_ret.copyTo(img_);
  }
  std::vector<std::vector<cv::Point2f> > aruco_corners;
  std::vector<int> aruco_ids;
  std::vector<yaed::Ellipse> ellipses;
  for (Target tgt : tgts_.targets)
  {
    cv::circle(img_, cv::Point(int(tgt.cx * tgts_.width), int(tgt.cy * tgts_.height)), 4, cv::Scalar(0,255,0), 2);
    if ((with_all || with_aruco) && tgt.has_aruco)
    {
      std::vector<cv::Point2f> a_corners;
      int a_id;
      if (tgt.getAruco(a_id, a_corners))  { aruco_ids.push_back(a_id); aruco_corners.push_back(a_corners); }
    }
    if ((with_all || with_box) && tgt.has_box)
    {
      Box b;
      tgt.getBox(b);
      cv::rectangle(img_, cv::Rect(b.x1, b.y1, b.x2-b.x1+1, b.y2-b.y1+1), cv::Scalar(0,0,255), 1, 1, 0);
      if ((with_all || with_category) && tgt.has_category)
      {
        cv::putText(img_, tgt.category, cv::Point(b.x1, b.y1-4), cv::FONT_HERSHEY_DUPLEX, 0.4, cv::Scalar(255,0,0));
      }
      if ((with_all || with_tid) && tgt.has_tid)
      {
        char tmp[32];
        sprintf(tmp, "TID: %d", tgt.tracked_id);
        cv::putText(img_, tmp, cv::Point(b.x1, b.y1-14), cv::FONT_HERSHEY_DUPLEX, 0.4, cv::Scalar(0,0,255));
      }
    }
    if ((with_all || with_ell) && tgt.has_ell)
    {
      double xc, yc, a, b, rad;
      if (tgt.getEllipse(xc, yc, a, b, rad))
      {
        ellipses.push_back(yaed::Ellipse(xc, yc, a, b, rad, tgt.score));
      }
    }
    if ((with_all || with_seg) && tgt.has_seg)
    {
      cv::Mat mask = tgt.getMask() * 255;
      cv::threshold(mask, mask, 127, 255, cv::THRESH_BINARY);
      mask.convertTo(mask, CV_8UC1);

      cv::resize(mask, mask, cv::Size(img_.cols, img_.rows));
      std::vector<std::vector<cv::Point> > contours;
      std::vector<cv::Vec4i> hierarchy;

      cv::findContours(mask, contours, hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
      cv::Mat mask_disp = img_.clone();
      cv::fillPoly(mask_disp, contours, cv::Scalar(255,255,255), cv::LINE_AA);
      cv::polylines(img_, contours, true, cv::Scalar(255,255,255), 2, cv::LINE_AA);

      double alpha = 0.6;
      cv::addWeighted(img_, alpha, mask_disp, 1.0-alpha, 0, img_);
    }
  }
  if ((with_all || with_aruco) && aruco_ids.size() > 0)
  {
    cv::aruco::drawDetectedMarkers(img_, aruco_corners, aruco_ids);
  }
  if ((with_all || with_ell) && ellipses.size() > 0)
  {
    yaed::EllipseDetector ed; ed.DrawDetectedEllipses(img_, ellipses);
  }
}

std::string Target::getJsonStr()
{
  std::string json_str = "{";
  char buf[1024];
  if (this->has_box)
  {
    sprintf(buf, "\"box\":[%d,%d,%d,%d],", _b_box.x1, _b_box.y1, _b_box.x2 - _b_box.x1, _b_box.y2 - _b_box.y1); // xywh
    json_str += std::string(buf);
  }
  if (this->has_ell)
  {
    sprintf(buf, "\"ell\":[%.3f,%.3f,%.3f,%.3f,%.3f],", this->_e_xc, this->_e_yc, this->_e_a, this->_e_b, this->_e_rad); // xyabr
    json_str += std::string(buf);
  }
  if (this->has_yaw)
  {
    sprintf(buf, "\"yaw\":%.3f,", this->yaw_a);
    json_str += std::string(buf);
  }
  if (this->has_los)
  {
    sprintf(buf, "\"los\":[%.3f,%.3f],", this->los_ax, this->los_ay);
    json_str += std::string(buf);
  }
  if (this->has_position)
  {
    sprintf(buf, "\"pos\":[%.3f,%.3f,%.3f],", this->px, this->py, this->pz);
    json_str += std::string(buf);
  }
  if (this->has_tid)
  {
    sprintf(buf, "\"tid\":%d,", this->tracked_id);
    json_str += std::string(buf);
  }
  if (this->has_category)
  {
    sprintf(buf, "\"cat\":\"%s\",", this->category.c_str());
    json_str += std::string(buf);
  }
  sprintf(buf, "\"sc\":%.3f,\"cet\":[%.3f,%.3f]}", this->score, this->cx, this->cy);
  json_str += std::string(buf);
  return json_str;
}

std::string TargetsInFrame::getJsonStr()
{
  std::string json_str = "{";
  char buf[1024];
  
  if (this->has_fps)
  {
    sprintf(buf, "\"fps\":%.3f,", this->fps);
    json_str += std::string(buf);
  }
  if (this->has_fov)
  {
    sprintf(buf, "\"fov\":[%.3f,%.3f],", this->fov_x, this->fov_y);
    json_str += std::string(buf);
  }
  if (this->has_pod_info)
  {
    sprintf(buf, "\"pod\":[%.3f,%.3f,%.3f],", this->pod_patch, this->pod_roll, this->pod_yaw);
    json_str += std::string(buf);
  }
  if (this->has_uav_pos)
  {
    sprintf(buf, "\"uav_pos\":[%.7f,%.7f,%.3f],", this->longitude, this->latitude, this->altitude);
    json_str += std::string(buf);
  }
  if (this->has_uav_vel)
  {
    sprintf(buf, "\"uav_vel\":[%.3f,%.3f,%.3f],", this->uav_vx, this->uav_vy, this->uav_vz);
    json_str += std::string(buf);
  }
  if (this->has_ill)
  {
    sprintf(buf, "\"ill\":%.3f,", this->illumination);
    json_str += std::string(buf);
  }
  if (this->date_captured.size() > 0)
  {
    sprintf(buf, "\"time\":\"%s\",", this->date_captured.c_str());
    json_str += std::string(buf);
  }
  
  json_str += "\"rois\":[";
  for (int i=0; (int)i<this->rois.size(); i++)
  {
    if (i == (int)this->rois.size() - 1)
    {
      sprintf(buf, "[%d,%d,%d,%d]", this->rois[i].x1, this->rois[i].y1, this->rois[i].x2-this->rois[i].x1, this->rois[i].y2-this->rois[i].y1);
    }
    else
    {
      sprintf(buf, "[%d,%d,%d,%d],", this->rois[i].x1, this->rois[i].y1, this->rois[i].x2-this->rois[i].x1, this->rois[i].y2-this->rois[i].y1);
    }
    json_str += std::string(buf);
  }
  json_str += "],";
  json_str += "\"tgts\":[";
  for (int i=0; (int)i<this->targets.size(); i++)
  {
    if (i == (int)this->targets.size() - 1)
    {
      json_str += this->targets[i].getJsonStr();
    }
    else
    {
      json_str += this->targets[i].getJsonStr() + ",";
    }
  }
  json_str += "],";
  sprintf(buf, "\"h\":%d,\"w\":%d,\"fid\":%d}", this->height, this->width, this->frame_id);
  json_str += std::string(buf);
  return json_str;
}

bool Target::getEllipse(double& xc_, double& yc_, double& a_, double& b_, double& rad_)
{
  xc_ = this->_e_xc;
  yc_ = this->_e_yc;
  a_ = this->_e_a;
  b_ = this->_e_b;
  rad_ = this->_e_rad;
  return this->has_ell;
}

bool Target::getAruco(int& id, std::vector<cv::Point2f> &corners)
{
  id = this->_a_id;
  corners = this->_a_corners;
  return this->has_aruco;
}

bool Target::getBox(Box& b)
{
  b = this->_b_box;
  return this->has_box;
}

void Target::setAruco(int id_, std::vector<cv::Point2f> corners_, cv::Vec3d rvecs_, cv::Vec3d tvecs_, int img_w_, int img_h_, cv::Mat camera_matrix_)
{
  this->_a_id = id_;
  this->_a_corners = corners_;
  this->_a_rvecs = rvecs_;
  this->_a_tvecs = tvecs_;

  double x_mid = (corners_[0].x + corners_[1].x) / 2.;
  double y_mid = (corners_[0].y + corners_[1].y) / 2.;
  
  double left = std::min(std::min(corners_[0].x, corners_[1].x), std::min(corners_[2].x, corners_[3].x));
  double right = std::max(std::max(corners_[0].x, corners_[1].x), std::max(corners_[2].x, corners_[3].x));
  double top = std::min(std::min(corners_[0].y, corners_[1].y), std::min(corners_[2].y, corners_[3].y));
  double bottom = std::max(std::max(corners_[0].y, corners_[1].y), std::max(corners_[2].y, corners_[3].y));

  double x_vec = x_mid - (left + right) / 2.;
  double y_vec = y_mid - (top + bottom) / 2.;

  this->setYaw(x_vec, y_vec);  
  this->setBox(left, top, right, bottom, img_w_, img_h_);
  
  this->score = 1.;
  char cate[256];
  sprintf(cate, "aruco-%d", id_);
  this->setCategory(cate, id_);
  this->setTrackID(id_);
  this->setLOS(this->cx, this->cy, camera_matrix_, img_w_, img_h_);
  this->setPosition(tvecs_[0], tvecs_[1], tvecs_[2]);
  
  this->has_aruco = true;
}

void Target::setYaw(double vec_x_, double vec_y_)
{
  if (vec_x_ == 0. && vec_y_ > 0.)
  {
    this->yaw_a = 180;
  }
  else if (vec_x_ == 0. && vec_y_ < 0.)
  {
    this->yaw_a = 0;
  }
  else if (vec_x_ > 0. && vec_y_ == 0.)
  {
    this->yaw_a = 90;
  }
  else if (vec_x_ > 0. && vec_y_ > 0.)
  {
    this->yaw_a = 180 - atan(vec_x_ / vec_y_) * SV_RAD2DEG;
  }
  else if (vec_x_ > 0. && vec_y_ < 0.)
  {
    this->yaw_a = atan(vec_x_ / -vec_y_) * SV_RAD2DEG;
  }
  else if (vec_x_ < 0. && vec_y_ == 0.)
  {
    this->yaw_a = -90;
  }
  else if (vec_x_ < 0. && vec_y_ > 0.)
  {
    this->yaw_a = atan(-vec_x_ / vec_y_) * SV_RAD2DEG - 180;
  }
  else if (vec_x_ < 0. && vec_y_ < 0.)
  {
    this->yaw_a = -atan(-vec_x_ / -vec_y_) * SV_RAD2DEG;
  }
  this->has_yaw = true;
}

void Target::setEllipse(double xc_, double yc_, double a_, double b_, double rad_, double score_, int img_w_, int img_h_, cv::Mat camera_matrix_, double radius_in_meter_)
{
  this->_e_xc = xc_;
  this->_e_yc = yc_;
  this->_e_a = a_;
  this->_e_b = b_;
  this->_e_rad = rad_;
  this->has_ell = true;
  
  this->score = score_;
  cv::Rect rect;
  yaed::Ellipse ell(xc_, yc_, a_, b_, rad_);
  ell.GetRectangle(rect);
  this->setBox(rect.x, rect.y, rect.x + rect.width, rect.y + rect.height, img_w_, img_h_);
  this->setCategory("ellipse", 0);
  this->setLOS(this->cx, this->cy, camera_matrix_, img_w_, img_h_);
  
  if (radius_in_meter_ > 0)
  {
    double z = camera_matrix_.at<double>(0, 0) * radius_in_meter_ / b_;
    double x = tan(this->los_ax / SV_RAD2DEG) * z;
    double y = tan(this->los_ay / SV_RAD2DEG) * z;
    this->setPosition(x, y, z);
  }
}

void Target::setLOS(double cx_, double cy_, cv::Mat camera_matrix_, int img_w_, int img_h_)
{
  this->los_ax = atan((cx_ * img_w_ - img_w_ / 2.) / camera_matrix_.at<double>(0, 0)) * SV_RAD2DEG;
  this->los_ay = atan((cy_ * img_h_ - img_h_ / 2.) / camera_matrix_.at<double>(1, 1)) * SV_RAD2DEG;
  this->has_los = true;
}

void Target::setCategory(std::string cate_, int cate_id_)
{
  this->category = cate_;
  this->category_id = cate_id_;
  this->has_category = true;
}

void Target::setTrackID(int id_)
{
  this->tracked_id = id_;
  this->has_tid = true;
}

void Target::setPosition(double x_, double y_, double z_)
{
  this->px = x_;
  this->py = y_;
  this->pz = z_;
  this->has_position = true;
}

void Target::setBox(int x1_, int y1_, int x2_, int y2_, int img_w_, int img_h_)
{
  this->_b_box.setXYXY(x1_, y1_, x2_, y2_);
  
  this->cx = (double)(x2_ + x1_) / 2 / img_w_;
  this->cy = (double)(y2_ + y1_) / 2 / img_h_;
  this->w = (double)(x2_ - x1_) / img_w_;
  this->h = (double)(y2_ - y1_) / img_h_;
  
  // std::cout << this->cx << ", " << this->cy << ", " << this->w << "," << this->h << std::endl;

  this->has_box = true;
  this->has_hw = true;
}

void Target::setMask(cv::Mat mask_)
{
  this->_mask = mask_;
  this->has_seg = true;
}
cv::Mat Target::getMask()
{
  return this->_mask;
}  

TargetsInFrame::TargetsInFrame(int frame_id_)
{
  this->frame_id = frame_id_;
  this->height = -1;
  this->width = -1;
  this->has_fps = false;
  this->has_fov = false;
  this->has_roi = false;
  this->has_pod_info = false;
  this->has_uav_pos = false;
  this->has_uav_vel = false;
  this->has_ill = false;
  this->type = MissionType::NONE;
}
void TargetsInFrame::setTimeNow()
{
  this->date_captured = _get_time_str();
}
void TargetsInFrame::setSize(int width_, int height_)
{
  this->width = width_;
  this->height = height_;
}
void TargetsInFrame::setFPS(double fps_)
{
  this->fps = fps_;
  this->has_fps = true;
}
void TargetsInFrame::setFOV(double fov_x_, double fov_y_)
{
  this->fov_x = fov_x_;
  this->fov_y = fov_y_;
  this->has_fov = true;
}

Box::Box()
{

}


void Box::setXYXY(int x1_, int y1_, int x2_, int y2_)
{
  x1 = x1_;
  y1 = y1_;
  x2 = x2_;
  y2 = y2_;
}

void Box::setXYWH(int x_, int y_, int w_, int h_)
{
  x1 = x_;
  y1 = y_;
  x2 = x_ + w_ - 1;
  y2 = y_ + h_ - 1;
}


VideoWriterBase::VideoWriterBase()
{
  this->_is_running = false;
  this->_fid = 0;
  this->_fcnt = 0;
}
VideoWriterBase::~VideoWriterBase()
{
  this->release();
  this->_tt.join();
}
cv::Size VideoWriterBase::getSize()
{
  return this->_image_size;
}
double VideoWriterBase::getFps()
{
  return this->_fps;
}
std::string VideoWriterBase::getFilePath()
{
  return this->_file_path;
}
bool VideoWriterBase::isRunning()
{
  return this->_is_running;
}
void VideoWriterBase::setup(std::string file_path, cv::Size size, double fps, bool with_targets)
{
  this->_file_path = file_path;
  this->_fps = fps;
  this->_image_size = size;
  this->_with_targets = with_targets;

  this->_init();
  
  this->_tt = std::thread(&VideoWriterBase::_run, this);
  this->_tt.detach();
}
void VideoWriterBase::write(cv::Mat image, TargetsInFrame tgts)
{
  if (this->_is_running)
  {
    cv::Mat image_put;
    if (this->_image_size.height == image.rows && this->_image_size.width == image.cols)
    {
      image.copyTo(image_put);
    }
    else
    {
      char msg[256];
      sprintf(msg, "SpireCV (106) Input image SIZE (%d, %d) != Saving SIZE (%d, %d)!", image.cols, image.rows, this->_image_size.width, this->_image_size.height);
      throw std::runtime_error(msg);
      // cv::resize(image, image_put, this->_image_size);
    }
    
    if (this->_targets_ofs)
    {
      this->_fid ++;
      image_put = _attach_aruco(this->_fid, image_put);
      tgts.frame_id = this->_fid;
      this->_tgts_to_write.push(tgts);
      if (this->_fid >= SV_MAX_FRAMES)
        this->_fid = 0;
    }
    this->_image_to_write.push(image_put);
  }
}
void VideoWriterBase::_run()
{
  while (this->_is_running && isOpenedImpl())
  {
    while (!this->_image_to_write.empty())
    {
      this->_fcnt ++;
      
      cv::Mat img = _image_to_write.front();
      if (this->_targets_ofs)
      {
        if (!this->_tgts_to_write.empty())
        {
          TargetsInFrame tgts = this->_tgts_to_write.front();
          
          std::string json_str = tgts.getJsonStr();
          _targets_ofs << json_str << std::endl;

          this->_tgts_to_write.pop();
        }
      }
      // this->_writer << img;
      writeImpl(img);
      this->_image_to_write.pop();

      if (this->_fcnt >= SV_MAX_FRAMES)
      {
        _init();
      }
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(int(1000 / this->_fps)));
  }
}

void VideoWriterBase::_init()
{
  this->release();

  // get now time
  time_t t = time(NULL);;
  tm* local = localtime(&t);

  char s_buf[128];
  strftime(s_buf, 64, "/FlyVideo_%Y-%m-%d_%H-%M-%S", local);
  std::string name = std::string(s_buf);
  
  bool opend = false;
  opend = setupImpl(name);

  if (!opend)
  {
    std::cout << "Failed to write video: " << _file_path + name << std::endl;
  }
  else
  {
    this->_is_running = true;
    if (this->_with_targets)
    {
      this->_targets_ofs.open(this->_file_path + name + ".svj");
      if (!this->_targets_ofs)
      {
        std::cout << "Failed to write info file: " << this->_file_path << std::endl;
        this->_is_running = false;
      }
    }
  }
}
void VideoWriterBase::release()
{
  this->_is_running = false;
  this->_fid = 0;
  this->_fcnt = 0;

  if (this->_targets_ofs.is_open())
    this->_targets_ofs.close();

  while (!this->_image_to_write.empty())
    this->_image_to_write.pop();
  while (!this->_tgts_to_write.empty())
    this->_tgts_to_write.pop();

  releaseImpl();
}
bool VideoWriterBase::setupImpl(std::string file_name_)
{
  return false;
}
bool VideoWriterBase::isOpenedImpl()
{
  return false;
}
void VideoWriterBase::writeImpl(cv::Mat img_)
{

}
void VideoWriterBase::releaseImpl()
{

}


CameraBase::CameraBase(CameraType type, int id)
{
  this->_is_running = false;
  this->_is_updated = false;
  this->_type = type;
  
  this->_width = -1;
  this->_height = -1;
  this->_fps = -1;
  this->_ip = "192.168.2.64";
  this->_port = -1;
  this->_brightness = -1;
  this->_contrast = -1;
  this->_saturation = -1;
  this->_hue = -1;
  this->_exposure = -1;

  this->open(type, id);
}
CameraBase::~CameraBase()
{
  this->_is_running = false;
  this->_tt.join();
}
void CameraBase::setWH(int width, int height)
{
  this->_width = width;
  this->_height = height;
}
void CameraBase::setFps(int fps)
{
  this->_fps = fps;
}
void CameraBase::setIp(std::string ip)
{
  this->_ip = ip;
}
void CameraBase::setPort(int port)
{
  this->_port = port;
}
void CameraBase::setBrightness(double brightness)
{
  this->_brightness = brightness;
}
void CameraBase::setContrast(double contrast)
{
  this->_contrast = contrast;
}
void CameraBase::setSaturation(double saturation)
{
  this->_saturation = saturation;
}
void CameraBase::setHue(double hue)
{
  this->_hue = hue;
}
void CameraBase::setExposure(double exposure)
{
  this->_exposure = exposure;
}

int CameraBase::getW()
{
  return this->_width;
}
int CameraBase::getH()
{
  return this->_height;
}
int CameraBase::getFps()
{
  return this->_fps;
}
std::string CameraBase::getIp()
{
  return this->_ip;
}
int CameraBase::getPort()
{
  return this->_port;
}
double CameraBase::getBrightness()
{
  return this->_brightness;
}
double CameraBase::getContrast()
{
  return this->_contrast;
}
double CameraBase::getSaturation()
{
  return this->_saturation;
}
double CameraBase::getHue()
{
  return this->_hue;
}
double CameraBase::getExposure()
{
  return this->_exposure;
}
bool CameraBase::isRunning()
{
  return this->_is_running;
}

void CameraBase::openImpl()
{
  if (this->_type == CameraType::WEBCAM)
  {
    this->_cap.open(this->_camera_id);
    if (this->_width > 0 && this->_height > 0)
    {
      this->_cap.set(cv::CAP_PROP_FRAME_WIDTH, this->_width);
      this->_cap.set(cv::CAP_PROP_FRAME_HEIGHT, this->_height);
    }
    if (this->_fps > 0)
    {
      this->_cap.set(cv::CAP_PROP_FPS, this->_fps);
    }
    if (this->_brightness > 0)
    {
      this->_cap.set(cv::CAP_PROP_BRIGHTNESS, this->_brightness);
    }
    if (this->_contrast > 0)
    {
      this->_cap.set(cv::CAP_PROP_CONTRAST, this->_contrast);
    }
    if (this->_saturation > 0)
    {
      this->_cap.set(cv::CAP_PROP_SATURATION, this->_saturation);
    }
    if (this->_hue > 0)
    {
      this->_cap.set(cv::CAP_PROP_HUE, this->_hue);
    }
    if (this->_exposure > 0)
    {
      this->_cap.set(cv::CAP_PROP_EXPOSURE, this->_exposure);
    }
  }
  else if (this->_type == CameraType::G1)
  {
    char pipe[512];
    if (this->_width <= 0 || this->_height <= 0)
    {
      this->_width = 1280;
      this->_height = 720;
    }
    if (this->_port <= 0)
    {
      this->_port = 554;
    }
    if (this->_fps <= 0)
    {
      this->_fps = 30;
    }
    // sprintf(pipe, "rtsp://%s:%d/H264?W=%d&H=%d&BR=10000000&FPS=%d", this->_ip.c_str(), this->_port, this->_width, this->_height, this->_fps);
    sprintf(pipe, "rtspsrc location=rtsp://%s:%d/H264?W=%d&H=%d&FPS=%d&BR=4000000 latency=100 ! application/x-rtp,media=video ! rtph264depay ! parsebin ! nvv4l2decoder enable-max-performancegst=1 ! nvvidconv ! video/x-raw,format=(string)BGRx ! videoconvert ! appsink sync=false", this->_ip.c_str(), this->_port, this->_width, this->_height, this->_fps);
    // std::cout << pipe << std::endl;
    // this->_cap.open(pipe);  // cv::CAP_GSTREAMER
    this->_cap.open(pipe, cv::CAP_GSTREAMER);
  }
}
void CameraBase::open(CameraType type, int id)
{
  this->_type = type;
  this->_camera_id = id;

  openImpl();

  if (this->_cap.isOpened())
  {
    std::cout << "Camera opened!" << std::endl;
    this->_is_running = true;
    this->_tt = std::thread(&CameraBase::_run, this);
    this->_tt.detach();
  }
  else if (type != CameraType::NONE)
  {
    std::cout << "Camera can NOT open!" << std::endl;
  }
}
void CameraBase::_run()
{
  while (this->_is_running && this->_cap.isOpened())
  {
    this->_cap >> this->_frame;
    this->_is_updated = true;
    std::this_thread::sleep_for(std::chrono::milliseconds(2));
  }
}
bool CameraBase::read(cv::Mat& image)
{
  if (this->_type == CameraType::WEBCAM || this->_type == CameraType::G1)
  {
    int n_try = 0;
    while (n_try < 5000)
    {
      if (this->_is_updated)
      {
        this->_is_updated = false;
        this->_frame.copyTo(image);
        break;
      }
      std::this_thread::sleep_for(std::chrono::milliseconds(20));
      n_try ++;
    }
  }
  if (image.cols == 0 || image.rows == 0)
  {
    throw std::runtime_error("SpireCV (101) Camera cannot OPEN, check CAMERA_ID!");
  }
  return image.cols > 0 && image.rows > 0;
}
void CameraBase::release()
{
  _cap.release();
}



VideoStreamerBase::VideoStreamerBase()
{
  this->_is_running = false;
}
VideoStreamerBase::~VideoStreamerBase()
{
  this->release();
}
cv::Size VideoStreamerBase::getSize()
{
  return this->_stream_size;
}
int VideoStreamerBase::getPort()
{
  return this->_port;
}
std::string VideoStreamerBase::getUrl()
{
  return this->_url;
}
int VideoStreamerBase::getBitrate()
{
  return this->_bitrate;
}
bool VideoStreamerBase::isRunning()
{
  return this->_is_running;
}

void VideoStreamerBase::_run()
{
  while (this->_is_running)
  {
    if (isOpenedImpl())
    {
      if (!this->_image_to_stream.empty())
      {
        cv::Mat img = this->_image_to_stream.top();
        writeImpl(img);
    
        while (!this->_image_to_stream.empty())
        {
          this->_image_to_stream.pop();
        }
      }
      std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }
    else
    {
      std::cout << "VideoStreamer.isOpened(): FALSE" << std::endl;
      std::this_thread::sleep_for(std::chrono::milliseconds(1000));
      setupImpl();
    }
  }
  std::cout << "streaming == FALSE" << std::endl;
}

void VideoStreamerBase::release()
{
  this->_is_running = false;
  releaseImpl();
}

void VideoStreamerBase::setup(cv::Size size, int port, int bitrate, std::string url)
{
  this->_bitrate = bitrate;
  this->_stream_size = size;

  this->_port = port;
  this->_url = url;

  if (setupImpl())
  {
    // std::cout << "stream ready at rtsp://127.0.0.1:" << this->_port << this->_url << std::endl;

    this->_is_running = true;
    this->_tt = std::thread(&VideoStreamerBase::_run, this);
    this->_tt.detach();
  }
}

void VideoStreamerBase::stream(cv::Mat image)
{
  if (this->_is_running)
  {
    cv::Mat image_stream;
    if (this->_stream_size.height == image.rows && this->_stream_size.width == image.cols)
      image.copyTo(image_stream);
    else
      cv::resize(image, image_stream, this->_stream_size);

    this->_image_to_stream.push(image_stream);
  }
}

bool VideoStreamerBase::setupImpl()
{
  return false;
}
bool VideoStreamerBase::isOpenedImpl()
{
  return false;
}
void VideoStreamerBase::writeImpl(cv::Mat image)
{

}

void VideoStreamerBase::releaseImpl()
{

}


}

