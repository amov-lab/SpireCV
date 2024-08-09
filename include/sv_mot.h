#ifndef __SV_MOT__
#define __SV_MOT__

#include "sv_core.h"
#include "sv_common_det.h"
#include <opencv2/opencv.hpp>
#include <opencv2/aruco.hpp>
#include <opencv2/tracking.hpp>
#include <string>
#include <chrono>
#include <Eigen/Dense>


namespace sv {


class SORT;
class BYTETracker;

class MultipleObjectTracker : public CameraAlgorithm
{
public:
    MultipleObjectTracker();
    ~MultipleObjectTracker();

    void init(CommonObjectDetector* detector_);
    sv::TargetsInFrame track(cv::Mat img_, TargetsInFrame& tgts_);

private:
    void _load();
    bool _params_loaded;

    std::string _algorithm;
    bool _with_reid;
    int _reid_input_h;
    int _reid_input_w;
    int _reid_num_samples;
    double _reid_match_thres;
    double _iou_thres;
    int _max_age;
    int _min_hits;
    int _frame_rate;
    int _track_buffer;
    
    SORT* _sort_impl;
    BYTETracker* _bytetrack_impl;
    CommonObjectDetector* _detector;
};
}
#endif
