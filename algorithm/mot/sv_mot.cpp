#include "sv_mot.h"
#include <cmath>
#include <fstream>
#include "gason.h"
#include "sv_util.h"

using namespace std;
using namespace Eigen;

namespace sv {


MultipleObjectTracker::MultipleObjectTracker()
{
    this->_params_loaded = false;
    this->_sort_impl = NULL;
}
MultipleObjectTracker::~MultipleObjectTracker()
{
    if (this->_sort_impl)
        delete this->_sort_impl;
}

void MultipleObjectTracker::track(cv::Mat img_, TargetsInFrame& tgts_)
{
    if (!this->_params_loaded)
    {
        this->_load();
        this->_params_loaded = true;
    }
    if ("sort" == this->_algorithm && this->_sort_impl)
    {
        this->_detector->detect(img_, tgts_);
        this->_sort_impl->update(tgts_);
    }
}

void MultipleObjectTracker::init(CommonObjectDetector* detector_)
{
    if (!this->_params_loaded)
    {
        this->_load();
        this->_params_loaded = true;
    }
    if ("sort" == this->_algorithm)
    {
        this->_sort_impl = new SORT(this->_iou_thres, this->_max_age, this->_min_hits);
    }
    this->_detector = detector_;
}

void MultipleObjectTracker::_load()
{
    JsonValue all_value;
    JsonAllocator allocator;
    _load_all_json(this->alg_params_fn, all_value, allocator);

    JsonValue tracker_params_value;
    _parser_algorithm_params("MultipleObjectTracker", all_value, tracker_params_value);

    for (auto i : tracker_params_value) {
        if ("algorithm" == std::string(i->key)) {
            this->_algorithm = i->value.toString();
            std::cout << "algorithm: " << this->_algorithm << std::endl;
        }
        else if ("with_reid" == std::string(i->key)) {
            if (i->value.getTag() == JSON_TRUE)
                this->_with_reid = true;
            else
                this->_with_reid = false;
            
            std::cout << "with_reid: " << this->_with_reid << std::endl;
        }
        else if ("reid_input_size" == std::string(i->key) && i->value.getTag() == JSON_ARRAY) {
            int jcnt = 0;
            for (auto j : i->value) {
                if (jcnt == 0) {
                    this->_reid_input_w = j->value.toNumber();
                }
                if (jcnt == 1) {
                    this->_reid_input_h = j->value.toNumber();
                }
                jcnt += 1;
            }
            std::cout << "reid_input_w: " << this->_reid_input_w << std::endl;
            std::cout << "reid_input_h: " << this->_reid_input_h << std::endl;
        }
        else if ("reid_num_samples" == std::string(i->key)) {
            this->_reid_num_samples = i->value.toNumber();
            std::cout << "reid_num_samples: " << this->_reid_num_samples << std::endl;
        }
        else if ("reid_match_thres" == std::string(i->key)) {
            this->_reid_match_thres = i->value.toNumber();
            std::cout << "reid_match_thres: " << this->_reid_match_thres << std::endl;
        }
        else if ("iou_thres" == std::string(i->key)) {
            this->_iou_thres = i->value.toNumber();
            std::cout << "iou_thres: " << this->_iou_thres << std::endl;
        }
        else if ("max_age" == std::string(i->key)) {
            this->_max_age = i->value.toNumber();
            std::cout << "max_age: " << this->_max_age << std::endl;
        }
        else if ("min_hits" == std::string(i->key)) {
            this->_min_hits = i->value.toNumber();
            std::cout << "min_hits: " << this->_min_hits << std::endl;
        }
    }
}



KalmanFilter::KalmanFilter()
{
    this->_chi2inv95 << 3.8415, 5.9915, 7.8147, 9.4877, 11.070, 12.592, 14.067, 15.507, 16.919;
    this->_F = MatrixXd::Identity(8, 8);
    for (int i=0; i<4; i++)
    {
        this->_F(i,i+4) = 1;
    }
    this->_H = MatrixXd::Identity(4, 8);
    this->_std_weight_position = 1. / 20;
    this->_std_weight_vel = 1. / 160;
}

KalmanFilter::~KalmanFilter()
{
}

pair<Matrix<double, 8, 1>, Matrix<double, 8, 8> > KalmanFilter::initiate(Vector4d &bbox)
{
    Matrix<double,8,1> mean;
    mean << bbox(0), bbox(1), bbox(2)/bbox(3), bbox(3), 0, 0, 0, 0;
    VectorXd stds(8);
    stds << 2 * this->_std_weight_position * mean(3), 2 * this->_std_weight_position * mean(3), 0.01, 2 * this->_std_weight_position * mean(3), \
        10 * this->_std_weight_vel * mean(3), 10 * this->_std_weight_vel * mean(3), 1e-5, 10 * this->_std_weight_vel * mean(3);
    MatrixXd squared = stds.array().square();
    Matrix<double, 8, 8> covariances;
    covariances = squared.asDiagonal();
    return make_pair(mean, covariances);
}

pair<Matrix<double, 8, 1>, Matrix<double, 8, 8> > KalmanFilter::update(Matrix<double, 8, 1> mean, Matrix<double, 8, 8> covariances, sv::Box &box)
{
    MatrixXd R;
    Vector4d stds;

    stds << this->_std_weight_position * mean(3), this->_std_weight_position * mean(3), 0.1, this->_std_weight_position * mean(3);
    MatrixXd squared = stds.array().square();
    R = squared.asDiagonal();
    MatrixXd S = this->_H * covariances * this->_H.transpose() + R;    

    MatrixXd Kalman_gain = covariances * this->_H.transpose() * S.inverse();
    VectorXd measurement(4);
    measurement << box.x1, box.y1, (box.x2-box.x1)/(box.y2-box.y1), box.y2-box.y1;
    Matrix<double, 8, 1> new_mean = mean + Kalman_gain * (measurement - this->_H * mean);
    Matrix<double, 8, 8> new_covariances = (MatrixXd::Identity(8, 8) - Kalman_gain * this->_H) * covariances;
    return make_pair(new_mean, new_covariances);    
}

pair<Matrix<double, 8, 1>, Matrix<double, 8, 8> > KalmanFilter::predict(Matrix<double, 8, 1> mean, Matrix<double, 8, 8> covariances)
{
    VectorXd stds(8);
    stds << this->_std_weight_position * mean(3), this->_std_weight_position * mean(3), 0.01, this->_std_weight_position * mean(3), \
        this->_std_weight_vel * mean(3), this->_std_weight_vel * mean(3), 1e-5, this->_std_weight_vel * mean(3);
    MatrixXd squared = stds.array().square();
    MatrixXd Q = squared.asDiagonal();
    Matrix<double, 8, 1> pre_mean = this->_F * mean;
    Matrix<double, 8, 8> pre_cov = this->_F * covariances * this->_F.transpose() + Q;
    return make_pair(pre_mean, pre_cov);
}


SORT::~SORT()
{
}

void SORT::update(TargetsInFrame& tgts)
{
    sv::KalmanFilter kf;
    if (! this->_tracklets.size())
    {
        Vector4d bbox;
        for (int i=0; i<tgts.targets.size(); i++)
        {
            sv::Box box;
            tgts.targets[i].getBox(box);		
            Tracklet tracklet;
            tracklet.id = ++ this->_next_tracklet_id;
            // cout << tracklet.id << endl;
            tgts.targets[i].tracked_id = this->_next_tracklet_id;

            tracklet.bbox << box.x1,box.y1,box.x2-box.x1,box.y2-box.y1;  // x,y,w,h
            tracklet.age = 0;
            tracklet.hits = 1;
            tracklet.misses = 0;
            // initate the motion
            pair<Matrix<double, 8, 1>, Matrix<double, 8, 8> > motion = kf.initiate(tracklet.bbox);
            tracklet.mean=motion.first;
            tracklet.covariance = motion.second;

            this->_tracklets.push_back(tracklet);
        }
    }
    else
    {
        for (int i=0; i<tgts.targets.size(); i++)
        {
            tgts.targets[i].tracked_id = 0;
        }

        array<int, 100> match_det;
        match_det.fill(-1);
        // predict the next state of each tracklet
        for (auto& tracklet : this->_tracklets)
        {
            tracklet.age++;
            pair<Matrix<double, 8, 1>, Matrix<double, 8, 8> > motion = kf.predict(tracklet.mean, tracklet.covariance);
            tracklet.bbox << motion.first(0), motion.first(1), motion.first(2) * motion.first(3), motion.first(3);
            tracklet.mean = motion.first;
            tracklet.covariance = motion.second;    
        }
        
        // Match the detections to the existing tracklets 
        // cout << "the num of targets: " << tgts.targets.size() << endl;
        // cout << "the num of tracklets: " << this->_tracklets.size() << endl;
        vector<vector<double> > iouMatrix(this->_tracklets.size(), vector<double> (tgts.targets.size(), 0));
        for (int i=0; i<this->_tracklets.size(); i++)
        {
            for (int j=0; j<tgts.targets.size(); j++)
            {
                sv::Box box;
                tgts.targets[j].getBox(box);
                iouMatrix[i][j] = this->_iou(this->_tracklets[i], box);
            }
        }
        vector<pair<int, int> > matches = this->_hungarian(iouMatrix);
        for (auto& match : matches)
        {
            int trackletIndex = match.first;
            int detectionIndex = match.second;
            if (trackletIndex >= 0 && detectionIndex >= 0)
            {
                if(iouMatrix[match.first][match.second] >= 0)
                {
                    sv::Box box;
                    tgts.targets[detectionIndex].getBox(box);
                    this->_tracklets[trackletIndex].age = 0;
                    this->_tracklets[trackletIndex].hits++;
                    this->_tracklets[trackletIndex].bbox << box.x1, box.y1, box.x2-box.x1, box.y2-box.y1;

                    auto[mean, covariance] = kf.update(this->_tracklets[trackletIndex].mean, this->_tracklets[trackletIndex].covariance, box);
                    this->_tracklets[trackletIndex].mean = mean;
                    this->_tracklets[trackletIndex].covariance = covariance;

                    tgts.targets[detectionIndex].tracked_id = this->_tracklets[trackletIndex].id;
                    match_det[detectionIndex] = detectionIndex;
                }
            }
        }
        // create new tracklets for unmatched detections
        for (int i = 0; i < tgts.targets.size(); i++)
        { 
            if (match_det[i]==-1)
            {
                sv::Box box;
                tgts.targets[i].getBox(box);
                Tracklet tracklet;
                tracklet.id = ++ this->_next_tracklet_id;
                tracklet.bbox << box.x1, box.y1, box.x2-box.x1, box.y2-box.y1;

                tracklet.age = 0;
                tracklet.hits = 1;
                tracklet.misses = 0;
                
                auto[new_mean, new_covariance] = kf.initiate(tracklet.bbox);
                tracklet.mean = new_mean;
                tracklet.covariance = new_covariance;
                
                tgts.targets[i].tracked_id = this->_next_tracklet_id;
                this->_tracklets.push_back(tracklet);
            }
        }
    }
}

vector<Tracklet> SORT::getTracklets() const
{
    return this->_tracklets;
}

double SORT::_iou(Tracklet& tracklet, sv::Box& box)
{
    double trackletX1 = tracklet.bbox(0);
    double trackletY1 = tracklet.bbox(1);
    double trackletX2 = tracklet.bbox(0) + tracklet.bbox(2);
    double trackletY2 = tracklet.bbox(1) + tracklet.bbox(3);

    double detectionX1 = box.x1;
    double detectionY1 = box.y1;
    double detectionX2 = box.x2;
    double detectionY2 = box.y2;
    double intersectionX1 = max(trackletX1, detectionX1);
    double intersectionY1 = max(trackletY1, detectionY1);
    double intersectionX2 = min(trackletX2, detectionX2);
    double intersectionY2 = min(trackletY2, detectionY2);

    double w = (intersectionX2-intersectionX1 > 0.0) ? (intersectionX2-intersectionX1) : 0.0;
    double h = (intersectionY2-intersectionY1 > 0.0) ? (intersectionY2-intersectionY1) : 0.0;
    double intersectionArea = w * h;

    double trackletArea = tracklet.bbox(2) * tracklet.bbox(3);

    double detectionArea = (box.x2-box.x1) * (box.y2-box.y1);
    double unionArea = trackletArea + detectionArea - intersectionArea;
    double iou = (1 - intersectionArea / unionArea * 1.0);

    return iou;
}

vector<pair<int, int> > SORT::_hungarian(vector<vector<double> > costMatrix)
{
    int numRows = costMatrix.size();
    int numCols = costMatrix[0].size();

    const bool transposed = numCols > numRows;
    // transpose the matrix if necessary
    if (transposed)
    {
        vector<vector<double> > transposedMatrix(numCols, vector<double>(numRows));
        for (int i=0; i<numRows; i++)
        {
            for (int j=0; j<numCols; j++)
            {
                transposedMatrix[j][i] = costMatrix[i][j];
            }
        }
        costMatrix = transposedMatrix;
        swap(numRows, numCols);
    }
    vector<double>rowMin(numRows, numeric_limits<double>::infinity());
    vector<double>colMin(numCols, numeric_limits<double>::infinity());
    vector<int>rowMatch(numRows, -1);
    vector<int>colMatch(numCols, -1);
    vector<pair<int, int> > matches;
    // step1: Subtract the row minimums from each row
    for (int i=0; i<numRows; i++)
    {
        for (int j=0; j<numCols; j++)
        {
            rowMin[i] = min(rowMin[i], costMatrix[i][j]);
        }
        for (int j=0; j<numCols; j++)
        {
            costMatrix[i][j] -= rowMin[i];
        }
    }
    // step2: substract the colcum minimums from each column
    for (int j=0; j<numCols; j++)
    {
        for (int i=0; i<numRows; i++)
        {
            colMin[j] = min(colMin[j], costMatrix[i][j]);
        }
        for (int i=0; i<numRows; i++)
        {
            costMatrix[i][j] -= colMin[j];
        }
    }
    // step3: find a maximal matching
    for (int i=0; i<numRows; i++) 
    {
        vector<bool> visited(numCols, false);
        _augment(costMatrix, i, rowMatch, colMatch, visited);
    }
    // step4: calculate the matches
    matches.clear();
    for (int j=0; j<numCols; j++)
    {
        matches.push_back(make_pair(colMatch[j], j));
    }
    if (transposed)
    {
        for (auto& match : matches)
        {
            swap(match.first,match.second);
        }
    }
    return matches;
}

bool SORT::_augment(const vector<vector<double> >& costMatrix, int row, vector<int>& rowMatch, vector<int>& colMatch, vector<bool>& visited) 
{
    int numCols = costMatrix[0].size();
    for (int j=0; j<numCols; j++)
    {
        if (costMatrix[row][j] == 0 && !visited[j])
        {
            visited[j] = true;
            if (colMatch[j] == -1 || _augment(costMatrix, colMatch[j], rowMatch, colMatch, visited))
            {
                rowMatch[row] = j;
                colMatch[j] = row;
                return true;
            }
        }
    }
    return false;
}


}

