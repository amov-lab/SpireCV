#ifndef __SV_SORT__
#define __SV_SORT__

#include "sv_core.h"
#include <opencv2/opencv.hpp>
#include <opencv2/aruco.hpp>
#include <opencv2/tracking.hpp>
#include <string>
#include <chrono>
#include <Eigen/Dense>


namespace sv {



// define the tracklet struct to store the tracked objects.
struct Tracklet
{
/* data */
public:
    Eigen::Vector4d bbox;  // double x, y, w, h;
    int id = 0;
    int age;
    int hits;
    int misses;
    int frame_id = 0;
    int category_id;
    bool tentative;
    std::vector<double> features;
    Eigen::Matrix<double, 8, 1> mean;
    Eigen::Matrix<double, 8, 8> covariance;
};


class KalmanFilter {
public:
    KalmanFilter();
    ~KalmanFilter();
    std::pair<Eigen::Matrix<double, 8, 1>, Eigen::Matrix<double, 8, 8> > initiate(Eigen::Vector4d &bbox);
    std::pair<Eigen::Matrix<double, 8, 1>, Eigen::Matrix<double, 8, 8> > update(Eigen::Matrix<double, 8, 1> mean, Eigen::Matrix<double, 8, 8> covariances, Box &box);
    std::pair<Eigen::Matrix<double, 8, 1>, Eigen::Matrix<double, 8, 8> > predict(Eigen::Matrix<double, 8, 1> mean, Eigen::Matrix<double, 8, 8> covariances);
    std::pair<Eigen::Matrix<double, 4, 1>, Eigen::Matrix<double, 4, 4> > project(Eigen::Matrix<double, 8, 1> mean, Eigen::Matrix<double, 8, 8> covariances);
private:
    Eigen::Matrix<double, 8, 8> _F;
    Eigen::Matrix<double, 4, 8> _H;
    Eigen::Matrix<double, 9, 1> _chi2inv95;
    double _std_weight_position;
    double _std_weight_vel;
};


class SORT {
public:
    SORT(double iou_threshold, int max_age, int min_hits): _iou_threshold(iou_threshold), _max_age(max_age), _min_hits(min_hits), _next_tracklet_id(0) {};
    ~SORT();
    void update(TargetsInFrame &tgts);
    std::vector<Tracklet> getTracklets() const;
private:
    double _iou(Tracklet &tracklet, Box &box);
    std::vector<std::pair<int,int>> _hungarian(std::vector<std::vector<double>> costMatrix);
    double _findMin(const std::vector<double>& vec);
    void _subtractMinFromRows(std::vector<std::vector<double>>& costMatrix);
    void _subtractMinFromCols(std::vector<std::vector<double>>& costMatrix);
    //bool _augment(const std::vector<std::vector<double>>& costMatrix, int row, std::vector<int>& rowMatch, std::vector<int>& colMatch, std::vector<bool>& visited);

    double _iou_threshold;
    int _max_age;
    int _min_hits;
    int _next_tracklet_id;
    std::vector <Tracklet> _tracklets;
    std::vector <Tracklet> _new_tracklets;
};


}
#endif
