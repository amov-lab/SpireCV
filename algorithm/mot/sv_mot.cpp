#include "sv_mot.h"
#include <cmath>
#include <fstream>
#include <limits>
#include <vector>
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
        this->_F(i,i+4) = 1.; //1
    }
    this->_H = MatrixXd::Identity(4, 8);
    this->_std_weight_position = 1. / 20; //1./20
    this->_std_weight_vel = 1. / 160; //1./160
}

KalmanFilter::~KalmanFilter()
{
}

pair<Matrix<double, 8, 1>, Matrix<double, 8, 8> > KalmanFilter::initiate(Vector4d &bbox)
{
    Matrix<double,8,1> mean;
    Matrix<double,4,1> zero_vector;
    zero_vector.setZero();
    mean << bbox(0), bbox(1), (double)bbox(2) / (double)bbox(3), bbox(3), zero_vector;
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
    VectorXd measurement(4);
    double a = (double)(box.x2-box.x1) / (double)(box.y2-box.y1);
    measurement << box.x1+(box.x2-box.x1)/2, box.y1+(box.y2-box.y1)/2, a, box.y2-box.y1;
    pair<Matrix<double, 4, 1>, Matrix<double, 4, 4> > projected = project(mean, covariances);
    Matrix<double, 4, 1> projected_mean = projected.first;
    Matrix<double, 4, 4> projected_cov = projected.second;

    Eigen::LLT<Eigen::MatrixXd> chol_factor(projected_cov);
    MatrixXd Kalman_gain = (chol_factor.solve((covariances * this->_H.transpose()).transpose())).transpose();
    
    VectorXd innovation = measurement - projected_mean;
    Matrix<double, 8, 1> new_mean = mean + Kalman_gain *innovation;
    Matrix<double, 8, 8> new_covariances = covariances - Kalman_gain * projected_cov * Kalman_gain.transpose();

    return make_pair(new_mean, new_covariances);
}

pair<Matrix<double, 4, 1>, Matrix<double, 4, 4> > KalmanFilter::project(Matrix<double, 8, 1> mean, Matrix<double, 8, 8> covariances)
{
    VectorXd stds(4);
    stds << this->_std_weight_position * mean(3), this->_std_weight_position * mean(3), 0.1, this->_std_weight_position * mean(3); 
    MatrixXd squared = stds.array().square();
    MatrixXd R = squared.asDiagonal();

    Matrix<double, 4, 1>  pro_mean = this->_H * mean;
    Matrix<double, 4, 4>  pro_covariances = this->_H * covariances * this->_H.transpose() + R;
    return make_pair(pro_mean, pro_covariances);
}
pair<Matrix<double, 8, 1>, Matrix<double, 8, 8> > KalmanFilter::predict(Matrix<double, 8, 1> mean, Matrix<double, 8, 8> covariances)
{
    VectorXd stds(8);
    stds << this->_std_weight_position * mean(3), this->_std_weight_position * mean(3), 1e-2, this->_std_weight_position * mean(3), \
        this->_std_weight_vel * mean(3), this->_std_weight_vel * mean(3), 1e-5, this->_std_weight_vel * mean(3); // a = 0.01
    MatrixXd squared = stds.array().square();
    MatrixXd Q = squared.asDiagonal();
    Matrix<double, 8, 1> pre_mean = this->_F * mean;
    Matrix<double, 8, 8> pre_cov = this->_F * covariances * this->_F.transpose()+Q;//+Q
    return make_pair(pre_mean, pre_cov);
}


SORT::~SORT()
{
}

void SORT::update(TargetsInFrame& tgts)
{
    sv::KalmanFilter kf;
    if (! this->_tracklets.size() || tgts.targets.size() == 0)
    {
        Vector4d bbox;
        for (int i=0; i<tgts.targets.size(); i++)
        {
            sv::Box box;
            tgts.targets[i].getBox(box);		
            Tracklet tracklet;
            tracklet.id = ++ this->_next_tracklet_id;
            tgts.targets[i].tracked_id = this->_next_tracklet_id;
            tgts.targets[i].has_tid = true;

            tracklet.bbox << box.x1+(box.x2-box.x1)/2, box.y1+(box.y2-box.y1)/2, box.x2-box.x1, box.y2-box.y1;  // x,y,w,h; center(x,y)
            
            tracklet.age = 0;
            tracklet.hits = 1;
            tracklet.misses = 0;
            tracklet.frame_id = tgts.frame_id;
            tracklet.category_id = tgts.targets[i].category_id;
            tracklet.tentative = true;
            
            // initate the motion
            pair<Matrix<double, 8, 1>, Matrix<double, 8, 8> > motion = kf.initiate(tracklet.bbox);
            tracklet.mean = motion.first;
            tracklet.covariance = motion.second;
            
            this->_tracklets.push_back(tracklet);
        }
    }
    else
    {
        // cout << "frame id:" << tgts.frame_id << endl;
        for (int i=0; i<tgts.targets.size(); i++)
        {
            tgts.targets[i].tracked_id = 0;
            tgts.targets[i].has_tid = true;
        }

        vector<int> match_det(tgts.targets.size(), -1);
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
                if (iouMatrix[match.first][match.second] <= 1-_iou_threshold)  // iou_thrshold
                {
                    sv::Box box;
                    tgts.targets[detectionIndex].getBox(box);
                    this->_tracklets[trackletIndex].age = 0;
                    this->_tracklets[trackletIndex].hits++;
                    this->_tracklets[trackletIndex].frame_id = tgts.frame_id;
                    this->_tracklets[trackletIndex].bbox << box.x1+(box.x2-box.x1)/2, box.y1+(box.y2-box.y1)/2, box.x2-box.x1, box.y2-box.y1;
                    tgts.targets[detectionIndex].tracked_id = this->_tracklets[trackletIndex].id;
                    match_det[detectionIndex] = trackletIndex;
                }
            }
        }
        std::vector <vector<double>> ().swap(iouMatrix);
        for (int i=0; i<tgts.targets.size(); i++)
        { 
            // cout << "match_det: index: " << i << " value: " << match_det[i] << endl;
            if (match_det[i] == -1)
            {
                // cout << "create new tracklet." << endl;
                sv::Box box;
                tgts.targets[i].getBox(box);
                Tracklet tracklet;
                tracklet.id = ++ this->_next_tracklet_id;
                tracklet.bbox << box.x1+(box.x2-box.x1)/2, (double)(box.y1+(box.y2-box.y1)/2), box.x2-box.x1, box.y2-box.y1;
                tracklet.age = 0;
                tracklet.hits = 1;
                tracklet.misses = 0;
                tracklet.frame_id = tgts.frame_id;
                tracklet.category_id = tgts.targets[i].category_id;
                tracklet.tentative = true;
                
                pair<Matrix<double, 8, 1>, Matrix<double, 8, 8> > new_motion = kf.initiate(tracklet.bbox);
                tracklet.mean = new_motion.first;
                tracklet.covariance = new_motion.second;

                tgts.targets[i].tracked_id = this->_next_tracklet_id;
                tgts.targets[i].has_tid = true;
                this->_tracklets.push_back(tracklet);
            }
            else
            {
                sv::Box box;
                int track_id = match_det[i];
                tgts.targets[i].getBox(box);
                pair<Matrix<double, 8, 1>, Matrix<double, 8, 8> > updated = kf.update(this->_tracklets[track_id].mean, this->_tracklets[track_id].covariance, box);
                this->_tracklets[track_id].mean = updated.first;
                this->_tracklets[track_id].covariance = updated.second;
            }
        }
        
        //sift tracklets
        for (auto& tracklet : this->_tracklets)
        {
            if (tracklet.hits >= _min_hits)
            {
                tracklet.tentative = false;
            }
            if ((tgts.frame_id-tracklet.frame_id <= _max_age) || (!tracklet.tentative && tracklet.frame_id == tgts.frame_id))
            {
                _new_tracklets.push_back(tracklet);
            } 
        }
        _tracklets = _new_tracklets;
        std::vector <Tracklet> ().swap(_new_tracklets);
    }
}

vector<Tracklet> SORT::getTracklets() const
{
    return this->_tracklets;
}

double SORT::_iou(Tracklet& tracklet, sv::Box& box)
{
    double trackletX1 = tracklet.bbox(0)-tracklet.bbox(2)/2;
    double trackletY1 = tracklet.bbox(1)-tracklet.bbox(3)/2;
    double trackletX2 = tracklet.bbox(0) + tracklet.bbox(2)/2;
    double trackletY2 = tracklet.bbox(1) + tracklet.bbox(3)/2;
    
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

// Function to find the minimum element in a vector
double SORT::_findMin(const std::vector<double>& vec) {
    double minVal = std::numeric_limits<double>::max();
    for (double val : vec) {
        if (val < minVal) {
            minVal = val;
        }
    }
    return minVal;
}

// Function to subtract the minimum value from each row of the cost matrix
void SORT::_subtractMinFromRows(std::vector<std::vector<double>>& costMatrix) {
    for (auto& row : costMatrix) {
        double minVal = _findMin(row);
        for (double& val : row) {
            val -= minVal;
        }
    }
}

// Function to subtract the minimum value from each column of the cost matrix
void SORT::_subtractMinFromCols(std::vector<std::vector<double>>& costMatrix) {
    for (size_t col = 0; col < costMatrix[0].size(); ++col) {
        double minVal = std::numeric_limits<double>::max();
        for (size_t row = 0; row < costMatrix.size(); ++row) {
            if (costMatrix[row][col] < minVal) {
                minVal = costMatrix[row][col];
            }
        }
        for (size_t row = 0; row < costMatrix.size(); ++row) {
            costMatrix[row][col] -= minVal;
        }
    }
}

// Function to find a matching using the Hungarian algorithm
vector<pair<int, int> > SORT::_hungarian(vector<vector<double> > costMatrix)
{
    size_t numRows = costMatrix.size();
    size_t numCols = costMatrix[0].size();

    //transpose the matrix if necessary
    const bool transposed = numCols > numRows;
    if (transposed) {
        vector<vector<double>> transposedMatrix(numCols, vector<double>(numRows));
        for (int i = 0; i < numRows; i++)
        {
            for (int j = 0; j < numCols; j++)
            {
                transposedMatrix[j][i] = costMatrix[i][j];
            }
        }
        costMatrix = transposedMatrix;
        swap(numRows, numCols);
    }
    // Determine the larger dimension for matching
    size_t maxDim = std::max(numRows, numCols);

    // Create a square cost matrix by padding with zeros if necessary
    std::vector<std::vector<double>> squareMatrix(maxDim, std::vector<double>(maxDim, 0.0));
    for (size_t row = 0; row < numRows; ++row) {
        for (size_t col = 0; col < numCols; ++col) {
            squareMatrix[row][col] = costMatrix[row][col];
        }
    }

    // Subtract the minimum value from each row and column
    _subtractMinFromRows(squareMatrix);
    _subtractMinFromCols(squareMatrix);

    // Initialize the assignment vectors with -1 values
    std::vector<int> rowAssignment(maxDim, -1);
    std::vector<int> colAssignment(maxDim, -1);

    // Perform the matching
    for (size_t row = 0; row < maxDim; ++row) {
        std::vector<bool> visitedCols(maxDim, false);
        for (size_t col = 0; col < maxDim; ++col) {
            if (squareMatrix[row][col] == 0 && colAssignment[col] == -1) {
                rowAssignment[row] = col;
                colAssignment[col] = row;
                break;
            }
        }
    }

    // Convert the assignment vectors to pair<int, int> format
    std::vector<std::pair<int, int>> assignmentPairs;
    for (size_t row = 0; row < numRows; ++row) {
        int col = rowAssignment[row];
        //if (col != -1) {
          //  assignmentPairs.emplace_back(row, col);
       // }
        if (col != -1) {
            if (col >= numCols) {
                col = -1;
            }
            assignmentPairs.emplace_back(row, col);
        }
    }
    if (transposed) {
        for (auto& assignment : assignmentPairs)
        {
            swap(assignment.first, assignment.second);
        }
    }
    return assignmentPairs;
    }
}

