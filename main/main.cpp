#include "packing/include/packing.hpp"
#include <iostream>
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/opencv.hpp>

uint8_t MAX_DIST_H = 10;
uint8_t MAX_DIST_S = 10;
uint8_t MAX_DIST_V = 10;

int main() {
  cv::Mat bgr_img = cv::imread("../benchmark.bmp", cv::IMREAD_COLOR);
  if (bgr_img.empty()) {
    std::cerr << "Failed to open image\n";
    return 1;
  }

  compressBRGImage(bgr_img, "something.bin", MAX_DIST_H, MAX_DIST_S,
                   MAX_DIST_V);
  cv::Mat output_img = uncompressBinary("something.bin");
  cv::imshow("Imagem: ", output_img);
  cv::waitKey(0);

  return 0;
}
