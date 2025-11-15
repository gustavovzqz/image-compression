#include <fstream>
#include <iostream>
#include <opencv2/opencv.hpp>

void exampleFileWrite() {

  std::ofstream file("bits.bin", std::ios::binary);

  int valor = 3;

  file.write(reinterpret_cast<char *>(&valor), sizeof(int));

  file.close();
}

int main() {
  // Load image
  cv::Mat bgr_img = cv::imread("../benchmark.bmp", cv::IMREAD_COLOR);
  if (bgr_img.empty()) {
    std::cerr << "Failed to open image\n";
    return 1;
  }

  exampleFileWrite();

  cv::Mat hls_image;
  cv::cvtColor(bgr_img, hls_image, cv::COLOR_BGR2HLS);

  cv::Mat img;
  cv::cvtColor(hls_image, img, cv::COLOR_HLS2BGR);

  // Display the image
  cv::imshow("Image", img);
  cv::waitKey(0);
  return 0;
}
