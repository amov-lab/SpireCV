#include "yolov7/cuda_utils.h"
#include "yolov7/logging.h"
#include "yolov7/utils.h"
#include "yolov7/preprocess.h"
#include "yolov7/postprocess.h"
#include "yolov7/model.h"

#include <iostream>
#include <chrono>
#include <cmath>

using namespace nvinfer1;

static Logger gLogger;
const static int kInputH = 640;
const static int kInputW = 640;
const static int kInputH_HD = 1280;
const static int kInputW_HD = 1280;
const static int kOutputSize = kMaxNumOutputBbox * sizeof(Detection) / sizeof(float) + 1;

bool parse_args(int argc, char** argv, std::string& wts, std::string& engine, bool& is_p6, float& gd, float& gw, std::string& img_dir, int& n_classes) {
  if (argc < 4) return false;
  if (std::string(argv[1]) == "-s" && (argc == 6 || argc == 8)) {
    wts = std::string(argv[2]);
    engine = std::string(argv[3]);
    n_classes = atoi(argv[4]);
    if (n_classes < 1)
      return false;
    auto net = std::string(argv[5]);
    if (net[0] == 'n') {
      gd = 0.33;
      gw = 0.25;
    } else if (net[0] == 's') {
      gd = 0.33;
      gw = 0.50;
    } else if (net[0] == 'm') {
      gd = 0.67;
      gw = 0.75;
    } else if (net[0] == 'l') {
      gd = 1.0;
      gw = 1.0;
    } else if (net[0] == 'x') {
      gd = 1.33;
      gw = 1.25;
    } else if (net[0] == 'c' && argc == 8) {
      gd = atof(argv[6]);
      gw = atof(argv[7]);
    } else {
      return false;
    }
    if (net.size() == 2 && net[1] == '6') {
      is_p6 = true;
    }
  } else {
    return false;
  }
  return true;
}

void serialize_engine(unsigned int max_batchsize, bool& is_p6, float& gd, float& gw, std::string& wts_name, std::string& engine_name, int n_classes) {
  // Create builder
  IBuilder* builder = createInferBuilder(gLogger);
  IBuilderConfig* config = builder->createBuilderConfig();

  // Create model to populate the network, then set the outputs and create an engine
  ICudaEngine *engine = nullptr;
  if (is_p6) {
    engine = build_det_p6_engine(max_batchsize, builder, config, DataType::kFLOAT, gd, gw, wts_name, kInputH_HD, kInputW_HD, n_classes);
  } else {
    engine = build_det_engine(max_batchsize, builder, config, DataType::kFLOAT, gd, gw, wts_name, kInputH, kInputW, n_classes);
  }
  assert(engine != nullptr);

  // Serialize the engine
  IHostMemory* serialized_engine = engine->serialize();
  assert(serialized_engine != nullptr);

  // Save engine to file
  std::ofstream p(engine_name, std::ios::binary);
  if (!p) {
    std::cerr << "Could not open plan output file" << std::endl;
    assert(false);
  }
  p.write(reinterpret_cast<const char*>(serialized_engine->data()), serialized_engine->size());

  // Close everything down
  engine->destroy();
  builder->destroy();
  config->destroy();
  serialized_engine->destroy();
}

int main(int argc, char** argv) {
  cudaSetDevice(kGpuId);

  std::string wts_name = "";
  std::string engine_name = "";
  bool is_p6 = false;
  float gd = 0.0f, gw = 0.0f;
  std::string img_dir;
  int n_classes;

  if (!parse_args(argc, argv, wts_name, engine_name, is_p6, gd, gw, img_dir, n_classes)) {
    std::cerr << "arguments not right!" << std::endl;
    std::cerr << "./SpireCVDet -s [.wts] [.engine] n_classes [n/s/m/l/x/n6/s6/m6/l6/x6 or c/c6 gd gw]  // serialize model to plan file" << std::endl;
    // std::cerr << "./SpireCVDet -d [.engine] ../images  // deserialize plan file and run inference" << std::endl;
    return -1;
  }
  std::cout << "n_classes: " << n_classes << std::endl;

  // Create a model using the API directly and serialize it to a file
  if (!wts_name.empty()) {
    serialize_engine(kBatchSize, is_p6, gd, gw, wts_name, engine_name, n_classes);
    return 0;
  }


  return 0;
}

