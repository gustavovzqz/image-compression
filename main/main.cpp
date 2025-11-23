#include "packing/include/packing.hpp"
#include <iostream>
#include <opencv2/core.hpp>
#include <opencv2/opencv.hpp>

int main() {
  cv::Mat bgr_img = cv::imread("../benchmark.bmp", cv::IMREAD_COLOR);
  if (bgr_img.empty()) {
    std::cerr << "Failed to open image\n";
    return 1;
  }

  writeImageBinaryHSV(bgr_img);
  readImageBinaryHSV();

  return 0;
}
