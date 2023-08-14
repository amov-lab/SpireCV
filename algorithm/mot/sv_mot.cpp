#include "sv_mot.h"
#include <cmath>
#include <fstream>
#include "gason.h"
#include "sv_util.h"


namespace sv {


MultipleObjectTracker::MultipleObjectTracker()
{
    this->_params_loaded = false;
}
MultipleObjectTracker::~MultipleObjectTracker()
{
}

void MultipleObjectTracker::track(cv::Mat img_, TargetsInFrame& tgts_)
{
    if (!this->_params_loaded)
    {
        this->_load();
        this->_params_loaded = true;
    }

}

void MultipleObjectTracker::init()
{
    if (!this->_params_loaded)
    {
        this->_load();
        this->_params_loaded = true;
    }
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

}

