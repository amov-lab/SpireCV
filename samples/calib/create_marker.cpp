#include <opencv2/highgui.hpp>
#include <opencv2/objdetect/aruco_detector.hpp>
#include <iostream>
#include "aruco_samples_utility.hpp"

using namespace cv;

namespace {
const char* about = "Create an ArUco marker image";

//! [aruco_create_markers_keys]
const char* keys  =
    "{@outfile |<none> | Output image }"
    "{d        |       | dictionary: DICT_4X4_50=0, DICT_4X4_100=1, DICT_4X4_250=2,"
    "DICT_4X4_1000=3, DICT_5X5_50=4, DICT_5X5_100=5, DICT_5X5_250=6, DICT_5X5_1000=7, "
    "DICT_6X6_50=8, DICT_6X6_100=9, DICT_6X6_250=10, DICT_6X6_1000=11, DICT_7X7_50=12,"
    "DICT_7X7_100=13, DICT_7X7_250=14, DICT_7X7_1000=15, DICT_ARUCO_ORIGINAL = 16}"
    "{cd       |       | Input file with custom dictionary }"
    "{id       |       | Marker id in the dictionary }"
    "{ms       | 200   | Marker size in pixels }"
    "{bs       | 50    | Border size in pixels }"
    "{lp       | 50    | Landing Pad Unit in pixels }"
    "{bb       | 1     | Number of bits in marker borders }"
    "{si       | false | show generated image }";
}
//! [aruco_create_markers_keys]


Mat create_marker_with_borders(aruco::Dictionary& dictionary, int markerId, int markerSize, int borderBits, int borderSize) {
  Mat tmpImg;
  aruco::generateImageMarker(dictionary, markerId, markerSize, tmpImg, borderBits);
  Mat tmpImgCopy = Mat::ones(borderSize * 2 + markerSize, borderSize * 2 + markerSize, CV_8UC1) * 255;
  tmpImg.copyTo(tmpImgCopy(Rect(borderSize, borderSize, markerSize, markerSize)));
  tmpImg = tmpImgCopy;
  return tmpImg;
}


int main(int argc, char *argv[]) {
  CommandLineParser parser(argc, argv, keys);
  parser.about(about);

  if(argc < 4) {
    parser.printMessage();
    return 0;
  }

  int markerId = parser.get<int>("id");
  int borderBits = parser.get<int>("bb");
  int markerSize = parser.get<int>("ms");
  bool showImage = parser.get<bool>("si");

  int borderSize = 0;
  if (parser.has("bs")) {
    borderSize = parser.get<int>("bs");
  }
  int landingPadUnit = 0;
  if (parser.has("lp")) {
    landingPadUnit = parser.get<int>("lp");
    borderSize = landingPadUnit;
    borderBits = 1;
    markerSize = landingPadUnit * 4;
  }

  String out = parser.get<String>(0);

  if(!parser.check()) {
    parser.printErrors();
    return 0;
  }

  aruco::Dictionary dictionary = aruco::getPredefinedDictionary(0);
  if (parser.has("d")) {
    int dictionaryId = parser.get<int>("d");
    dictionary = aruco::getPredefinedDictionary(aruco::PredefinedDictionaryType(dictionaryId));
  }
  else if (parser.has("cd")) {
    FileStorage fs(parser.get<std::string>("cd"), FileStorage::READ);
    bool readOk = dictionary.aruco::Dictionary::readDictionary(fs.root());
    if(!readOk) {
      std::cerr << "Invalid dictionary file" << std::endl;
      return 0;
    }
  }
  else {
    std::cerr << "Dictionary not specified" << std::endl;
    return 0;
  }

  Mat markerImg;
  aruco::generateImageMarker(dictionary, markerId, markerSize, markerImg, borderBits);
  if (borderSize > 0) {
    Mat markerImgCopy = Mat::ones(borderSize * 2 + markerSize, borderSize * 2 + markerSize, CV_8UC1) * 255;
    markerImg.copyTo(markerImgCopy(Rect(borderSize, borderSize, markerSize, markerSize)));
    markerImg = markerImgCopy;
  }

  if (landingPadUnit > 0) {
    Mat markerImgBIG = Mat::ones(landingPadUnit * 62, landingPadUnit * 62, CV_8UC1) * 255;
    // markerId = 0;
    markerId --;
    markerSize = landingPadUnit * 4;
    borderSize = landingPadUnit;
    int newSize = markerSize + borderSize * 2;

    for (int i=0; i<3; i++) {
      markerId += 1;
      markerImg = create_marker_with_borders(dictionary, markerId, markerSize, borderBits, borderSize);
      markerImg.copyTo(markerImgBIG(Rect(i * landingPadUnit * 5, 0, newSize, newSize)));
    }
    for (int i=0; i<5; i++) {
      markerId += 1;
      markerImg = create_marker_with_borders(dictionary, markerId, markerSize, borderBits, borderSize);
      markerImg.copyTo(markerImgBIG(Rect(landingPadUnit * 18 + i * landingPadUnit * 5, 0, newSize, newSize)));
    }
    for (int i=0; i<3; i++) {
      markerId += 1;
      markerImg = create_marker_with_borders(dictionary, markerId, markerSize, borderBits, borderSize);
      markerImg.copyTo(markerImgBIG(Rect(landingPadUnit * 46 + i * landingPadUnit * 5, 0, newSize, newSize)));
    }
    
    for (int i=0; i<2; i++) {
      markerId += 1;
      markerImg = create_marker_with_borders(dictionary, markerId, markerSize, borderBits, borderSize);
      markerImg.copyTo(markerImgBIG(Rect(landingPadUnit * 56, landingPadUnit * 5 + i * landingPadUnit * 5, newSize, newSize)));
    }
    for (int i=0; i<5; i++) {
      markerId += 1;
      markerImg = create_marker_with_borders(dictionary, markerId, markerSize, borderBits, borderSize);
      markerImg.copyTo(markerImgBIG(Rect(landingPadUnit * 56, landingPadUnit * 18 + i * landingPadUnit * 5, newSize, newSize)));
    }
    for (int i=0; i<3; i++) {
      markerId += 1;
      markerImg = create_marker_with_borders(dictionary, markerId, markerSize, borderBits, borderSize);
      markerImg.copyTo(markerImgBIG(Rect(landingPadUnit * 56, landingPadUnit * 46 + i * landingPadUnit * 5, newSize, newSize)));
    }
    
    for (int i=0; i<2; i++) {
      markerId += 1;
      markerImg = create_marker_with_borders(dictionary, markerId, markerSize, borderBits, borderSize);
      markerImg.copyTo(markerImgBIG(Rect(landingPadUnit * 51 - i * landingPadUnit * 5, landingPadUnit * 56, newSize, newSize)));
    }
    for (int i=0; i<5; i++) {
      markerId += 1;
      markerImg = create_marker_with_borders(dictionary, markerId, markerSize, borderBits, borderSize);
      markerImg.copyTo(markerImgBIG(Rect(landingPadUnit * 38 - i * landingPadUnit * 5, landingPadUnit * 56, newSize, newSize)));
    }
    for (int i=0; i<3; i++) {
      markerId += 1;
      markerImg = create_marker_with_borders(dictionary, markerId, markerSize, borderBits, borderSize);
      markerImg.copyTo(markerImgBIG(Rect(landingPadUnit * 10 - i * landingPadUnit * 5, landingPadUnit * 56, newSize, newSize)));
    }
    
    for (int i=0; i<2; i++) {
      markerId += 1;
      markerImg = create_marker_with_borders(dictionary, markerId, markerSize, borderBits, borderSize);
      markerImg.copyTo(markerImgBIG(Rect(0, landingPadUnit * 51 - i * landingPadUnit * 5, newSize, newSize)));
    }
    for (int i=0; i<5; i++) {
      markerId += 1;
      markerImg = create_marker_with_borders(dictionary, markerId, markerSize, borderBits, borderSize);
      markerImg.copyTo(markerImgBIG(Rect(0, landingPadUnit * 38 - i * landingPadUnit * 5, newSize, newSize)));
    }
    for (int i=0; i<2; i++) {
      markerId += 1;
      markerImg = create_marker_with_borders(dictionary, markerId, markerSize, borderBits, borderSize);
      markerImg.copyTo(markerImgBIG(Rect(0, landingPadUnit * 10 - i * landingPadUnit * 5, newSize, newSize)));
    }
    
    
    markerId += 1;
    markerImg = create_marker_with_borders(dictionary, markerId, markerSize, borderBits, borderSize);
    markerImg.copyTo(markerImgBIG(Rect(landingPadUnit * 28, landingPadUnit * 28, newSize, newSize)));
    markerId += 1;
    markerImg = create_marker_with_borders(dictionary, markerId, markerSize, borderBits, borderSize);
    markerImg.copyTo(markerImgBIG(Rect(landingPadUnit * 28, landingPadUnit * 23, newSize, newSize)));
    markerId += 1;
    markerImg = create_marker_with_borders(dictionary, markerId, markerSize, borderBits, borderSize);
    markerImg.copyTo(markerImgBIG(Rect(landingPadUnit * 33, landingPadUnit * 28, newSize, newSize)));
    markerId += 1;
    markerImg = create_marker_with_borders(dictionary, markerId, markerSize, borderBits, borderSize);
    markerImg.copyTo(markerImgBIG(Rect(landingPadUnit * 28, landingPadUnit * 33, newSize, newSize)));
    markerId += 1;
    markerImg = create_marker_with_borders(dictionary, markerId, markerSize, borderBits, borderSize);
    markerImg.copyTo(markerImgBIG(Rect(landingPadUnit * 23, landingPadUnit * 28, newSize, newSize)));
    
    markerId += 1;
    markerImg = create_marker_with_borders(dictionary, markerId, markerSize, borderBits, borderSize);
    markerImg.copyTo(markerImgBIG(Rect(landingPadUnit * 28, landingPadUnit * 18, newSize, newSize)));
    markerId += 1;
    markerImg = create_marker_with_borders(dictionary, markerId, markerSize, borderBits, borderSize);
    markerImg.copyTo(markerImgBIG(Rect(landingPadUnit * 38, landingPadUnit * 28, newSize, newSize)));
    markerId += 1;
    markerImg = create_marker_with_borders(dictionary, markerId, markerSize, borderBits, borderSize);
    markerImg.copyTo(markerImgBIG(Rect(landingPadUnit * 28, landingPadUnit * 38, newSize, newSize)));
    markerId += 1;
    markerImg = create_marker_with_borders(dictionary, markerId, markerSize, borderBits, borderSize);
    markerImg.copyTo(markerImgBIG(Rect(landingPadUnit * 18, landingPadUnit * 28, newSize, newSize)));
    
    markerId += 1;
    markerImg = create_marker_with_borders(dictionary, markerId, markerSize, borderBits, borderSize);
    markerImg.copyTo(markerImgBIG(Rect(landingPadUnit * 28, landingPadUnit * 5, newSize, newSize)));
    markerId += 1;
    markerImg = create_marker_with_borders(dictionary, markerId, markerSize, borderBits, borderSize);
    markerImg.copyTo(markerImgBIG(Rect(landingPadUnit * 51, landingPadUnit * 28, newSize, newSize)));
    markerId += 1;
    markerImg = create_marker_with_borders(dictionary, markerId, markerSize, borderBits, borderSize);
    markerImg.copyTo(markerImgBIG(Rect(landingPadUnit * 28, landingPadUnit * 51, newSize, newSize)));
    markerId += 1;
    markerImg = create_marker_with_borders(dictionary, markerId, markerSize, borderBits, borderSize);
    markerImg.copyTo(markerImgBIG(Rect(landingPadUnit * 5, landingPadUnit * 28, newSize, newSize)));
    
    markerId += 1;
    markerImg = create_marker_with_borders(dictionary, markerId, markerSize, borderBits, borderSize);
    markerImg.copyTo(markerImgBIG(Rect(landingPadUnit * 28, landingPadUnit * 10, newSize, newSize)));
    markerId += 1;
    markerImg = create_marker_with_borders(dictionary, markerId, markerSize, borderBits, borderSize);
    markerImg.copyTo(markerImgBIG(Rect(landingPadUnit * 46, landingPadUnit * 28, newSize, newSize)));
    markerId += 1;
    markerImg = create_marker_with_borders(dictionary, markerId, markerSize, borderBits, borderSize);
    markerImg.copyTo(markerImgBIG(Rect(landingPadUnit * 28, landingPadUnit * 46, newSize, newSize)));
    markerId += 1;
    markerImg = create_marker_with_borders(dictionary, markerId, markerSize, borderBits, borderSize);
    markerImg.copyTo(markerImgBIG(Rect(landingPadUnit * 10, landingPadUnit * 28, newSize, newSize)));
    
    // markerId = 90;
    markerSize = landingPadUnit * 4 * 4;
    borderSize = landingPadUnit * 4;
    newSize = markerSize + borderSize * 2;
    markerId += 1;
    markerImg = create_marker_with_borders(dictionary, markerId, markerSize, borderBits, borderSize);
    markerImg.copyTo(markerImgBIG(Rect(landingPadUnit * 5, landingPadUnit * 5, newSize, newSize)));
    markerId += 1;
    markerImg = create_marker_with_borders(dictionary, markerId, markerSize, borderBits, borderSize);
    markerImg.copyTo(markerImgBIG(Rect(landingPadUnit * 5 + newSize + landingPadUnit * 4, landingPadUnit * 5, newSize, newSize)));
    markerId += 1;
    markerImg = create_marker_with_borders(dictionary, markerId, markerSize, borderBits, borderSize);
    markerImg.copyTo(markerImgBIG(Rect(landingPadUnit * 5, landingPadUnit * 5 + newSize + landingPadUnit * 4, newSize, newSize)));
    markerId += 1;
    markerImg = create_marker_with_borders(dictionary, markerId, markerSize, borderBits, borderSize);
    markerImg.copyTo(markerImgBIG(Rect(landingPadUnit * 5 + newSize + landingPadUnit * 4, landingPadUnit * 5 + newSize + landingPadUnit * 4, newSize, newSize)));

    markerImg = Mat::ones(landingPadUnit * 64, landingPadUnit * 64, CV_8UC1) * 255;
    markerImgBIG.copyTo(markerImg(Rect(landingPadUnit, landingPadUnit, landingPadUnit * 62, landingPadUnit * 62)));
  }

  if(showImage) {
    imshow("marker", markerImg);
    waitKey(0);
  }

  imwrite(out, markerImg);

  return 0;
}

