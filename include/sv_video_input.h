#ifndef __SV_VIDEO_INPUT__
#define __SV_VIDEO_INPUT__

#include "sv_core.h"
#include <opencv2/opencv.hpp>
#include <opencv2/aruco.hpp>
#include <opencv2/tracking.hpp>
#include <string>
#include <chrono>


namespace sv {


class Camera : public CameraBase
{
public:
  Camera();
  ~Camera();
protected:
  void openImpl();

};


}
#endif
