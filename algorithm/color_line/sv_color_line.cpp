#include "sv_color_line.h"
#include "gason.h"
#include "sv_util.h"
#include <cmath>
#include <fstream>

namespace sv
{



ColorLineDetector::ColorLineDetector()
{
    this->is_load_parameter = false;
}

ColorLineDetector::~ColorLineDetector()
{
}

void ColorLineDetector::_load()
{
    JsonValue all_value;
    JsonAllocator allocator;
    _load_all_json(this->alg_params_fn, all_value, allocator);

    JsonValue colorliner_params_value;
    _parser_algorithm_params("ColorLineDetector", all_value, colorliner_params_value);

    for (auto i : colorliner_params_value)
    {
        if ("line_color" == std::string(i->key))
        {
            this->line_color = i->value.toString();
            std::cout << "line_color: " << this->line_color << std::endl;
        }
        else if ("line_location" == std::string(i->key))
        {
            this->line_location = i->value.toNumber();
        }
        else if ("line_location_a1" == std::string(i->key))
        {
            this->line_location_a1 = i->value.toNumber();
        }
        else if ("line_location_a2" == std::string(i->key))
        {
            this->line_location_a2 = i->value.toNumber();
        }
    }
}

void ColorLineDetector::get_line_area(cv::Mat &frame_, cv::Mat &line_area_, cv::Mat &line_area_a1_, cv::Mat &line_area_a2_)
{

    int h = frame_.rows;
    _half_h = h / 2.0;
    _half_w = frame_.cols / 2.0;
    int l1 = int(h * (1 - line_location - 0.05));
    int l2 = int(h * (1 - line_location));
    line_area_ = frame_(cv::Range(l1, l2), cv::Range::all());

    l1 = int(h * (1 - line_location_a1 - 0.05));
    l2 = int(h * (1 - line_location_a1));
    line_area_a1_ = frame_(cv::Range(l1, l2), cv::Range::all());
    _cy_a1 = l1;

    l1 = int(h * (1 - line_location_a2 - 0.05));
    l2 = int(h * (1 - line_location_a2));
    _cy_a2 = l1;
    line_area_a2_ = frame_(cv::Range(l1, l2), cv::Range::all());
}

float ColorLineDetector::cnt_area(std::vector<cv::Point> cnt_)
{
    float area = cv::contourArea(cnt_);
    return area;
}

void ColorLineDetector::seg(cv::Mat line_area_, cv::Mat line_area_a1_, cv::Mat line_area_a2_, std::string line_color_, cv::Point &center_, int &area_, cv::Point &center_a1_, cv::Point &center_a2_)
{
    int hmin, smin, vmin, hmax, smax, vmax;
    if (line_color_ == "black")
    {
        hmin = 0;
        smin = 0;
        vmin = 0;
        hmax = 180;
        smax = 255;
        vmax = 46;
    }
    else if (line_color_ == "red")
    {
        hmin = 0;
        smin = 43;
        vmin = 46;
        hmax = 10;
        smax = 255;
        vmax = 255;
    }
    else if (line_color_ == "yellow")
    {
        hmin = 26;
        smin = 43;
        vmin = 46;
        hmax = 34;
        smax = 255;
        vmax = 255;
    }
    else if (line_color_ == "green")
    {
        hmin = 35;
        smin = 43;
        vmin = 46;
        hmax = 77;
        smax = 255;
        vmax = 255;
    }
    else if (line_color_ == "blue")
    {
        hmin = 100;
        smin = 43;
        vmin = 46;
        hmax = 124;
        smax = 255;
        vmax = 255;
    }
    else
    {
        hmin = 0;
        smin = 0;
        vmin = 0;
        hmax = 180;
        smax = 255;
        vmax = 46;
    }

    cv::cvtColor(line_area_, line_area_, cv::COLOR_BGR2HSV);
    cv::inRange(line_area_, cv::Scalar(hmin, smin, vmin), cv::Scalar(hmax, smax, vmax), line_area_);

    cv::Mat kernel = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(5, 5));
    cv::morphologyEx(line_area_, line_area_, cv::MORPH_OPEN, kernel);

    std::vector<std::vector<cv::Point>> contours;
    std::vector<cv::Vec4i> hierarchy;
    cv::findContours(line_area_, contours, hierarchy, cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE);

    if (contours.size() > 0)
    {
        cv::Rect rect = cv::boundingRect(contours[0]);
        int cx = rect.x + rect.width / 2;
        int cy = rect.y + rect.height / 2;
        std::sort(contours.begin(), contours.end(),[](const std::vector<cv::Point> &a, const std::vector<cv::Point> &b)
                  {return cv::contourArea(a) > cv::contourArea(b);});
        area_ = cnt_area(contours[0]);
        center_ = cv::Point(cx, cy);
    }

    cv::cvtColor(line_area_a1_, line_area_a1_, cv::COLOR_BGR2HSV);
    cv::inRange(line_area_a1_, cv::Scalar(hmin, smin, vmin), cv::Scalar(hmax, smax, vmax), line_area_a1_);

    //cv2.MORPH_CLOSE 先进行膨胀，再进行腐蚀操作
    kernel = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(5, 5));
    cv::morphologyEx(line_area_a1_, line_area_a1_, cv::MORPH_CLOSE, kernel);

    std::vector<std::vector<cv::Point>> contours_a1;
    cv::findContours(line_area_a1_, contours_a1, hierarchy, cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE);

    if (contours_a1.size() > 0){
        cv::Rect rect = cv::boundingRect(contours_a1[0]);
        int cx = rect.x + rect.width / 2;
        int cy = rect.y + rect.height / 2 + _cy_a1;
        center_a1_ = cv::Point(cx - _half_w, cy - _half_h);
    }

    cv::cvtColor(line_area_a2_, line_area_a2_, cv::COLOR_BGR2HSV);
    cv::inRange(line_area_a2_, cv::Scalar(hmin, smin, vmin), cv::Scalar(hmax, smax, vmax), line_area_a2_);

    kernel = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(5, 5));
    cv::morphologyEx(line_area_a2_, line_area_a2_, cv::MORPH_CLOSE, kernel);

    std::vector<std::vector<cv::Point>> contours_a2;
    cv::findContours(line_area_a2_, contours_a2, hierarchy, cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE);

    if (contours_a2.size() > 0)
    {
        cv::Rect rect = cv::boundingRect(contours_a2[0]);
        int cx = rect.x + rect.width / 2;
        int cy = rect.y + rect.height / 2 + _cy_a2;
        center_a2_ = cv::Point(cx - _half_w, cy - _half_h);
    }
}

void ColorLineDetector::detect(cv::Mat img_, sv::TargetsInFrame &tgts_)
{

    if (!this->is_load_parameter)
    {
        _load();

        this->is_load_parameter = true;
    }
    int area_n = -1;
    cv::Mat area_base, area_base_a1, area_base_a2;
    cv::Point cxcy_n(0, 0), center_a1_n(0, 0), center_a2_n(0, 0);

    get_line_area(img_, area_base, area_base_a1, area_base_a2);
    seg(area_base, area_base_a1, area_base_a2, line_color, cxcy_n, area_n, center_a1_n, center_a2_n);
    pose.x = 0.0;
    pose.y = -1.0;
    pose.z = 0.0;

    if (area_n > 0)
    {
        circle(area_base, cv::Point(cxcy_n.x, cxcy_n.y), 4, cv::Scalar(0, 0, 255), -1);
        double angle = (cxcy_n.x - this->camera_matrix.at<double>(0, 2)) / this->camera_matrix.at<double>(0, 2) * atan((double)(area_base.rows / 2) / this->fov_x);
        pose.x = angle;
        pose.y = 1.0;
    }
    else
    {
        cv::Point cxcy__n(0, 0), center_a1__n(0, 0), center_a2__n(0, 0);
        seg(area_base, area_base_a1, area_base_a2, line_color, cxcy__n, area_n = 0, center_a1__n, center_a2__n);
        if (area_n > 0)
        {
            circle(area_base, cv::Point(cxcy_n.x, cxcy_n.y), 4, cv::Scalar(0, 0, 255), -1);
            double angle = (cxcy_n.x - this->camera_matrix.at<double>(0, 2)) / this->camera_matrix.at<double>(0, 2) * atan((double)(area_base.rows / 2) / this->fov_x);
            pose.x = angle;
            pose.y = 1.0;
            pose.z = 0.0;
        }
    }

    tgts_.setSize(img_.cols, img_.rows);
    tgts_.setFOV(this->fov_x, this->fov_y);
    auto t1 = std::chrono::system_clock::now();
    tgts_.setFPS(1000.0 / std::chrono::duration_cast<std::chrono::milliseconds>(t1 - this->_t0).count());
    this->_t0 = std::chrono::system_clock::now();
    tgts_.setTimeNow();

    if (area_n > 0)
    {
        Target tgt;
        tgt.los_ax = pose.x;
        if (cxcy_n.x != 0 || cxcy_n.y != 0)
        {
            tgt.cx = cxcy_n.x;
            tgt.cy = cxcy_n.y;
        }
        else if (center_a1_n.x != 0 || center_a1_n.y != 0)
        {
            tgt.cx = center_a1_n.x;
            tgt.cy = center_a1_n.y;
        }
        else if (center_a2_n.x != 0 || center_a2_n.y != 0)
        {
            tgt.cx = center_a2_n.x;
            tgt.cy = center_a2_n.y;
        }

        tgts_.targets.push_back(tgt);
    }
}
}
