#include <iostream>
#include <opencv2/opencv.hpp>
#include <vector>

int main() {
  // Load image
  cv::Mat img = cv::imread("benchmark.bmp", cv::IMREAD_COLOR);
  if (img.empty()) {
    std::cerr << "Failed to open image\n";
    return 1;
  }

  // Display the image
  cv::imshow("Image", img);
  cv::waitKey(0);
  return 0;
}
