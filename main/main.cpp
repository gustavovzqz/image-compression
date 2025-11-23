#include <fstream>
#include <iostream>
#include <opencv2/core.hpp>
#include <opencv2/opencv.hpp>

uint16_t packHSV(uchar h, uchar s, uchar v) {
  uint16_t H6 = (uint16_t)(h * 63 / 179);
  uint16_t S4 = (uint16_t)(s >> 4);
  uint16_t V6 = (uint16_t)(v >> 2);

  uint16_t packed = (H6 << 10) | (S4 << 6) | V6;
  return packed;
}

cv::Vec3b unpackHSV(uint16_t packed) {
  uint16_t H6 = (packed >> 10) & 0x3F;
  uint16_t S4 = (packed >> 6) & 0x0F;
  uint16_t V6 = (packed) & 0x3F;

  uchar h = (uchar)(H6 * 179 / 63);
  uchar s = (uchar)(S4 << 4);
  uchar v = (uchar)(V6 << 2);

  return cv::Vec3b(h, s, v);
}

void readImageBinaryHSV() {

  std::ifstream file("bits.bin", std::ios::binary);
  if (!file) {
    std::cerr << "Error opening file." << std::endl;
    return;
  }

  int width, height;
  file.read(reinterpret_cast<char *>(&width), sizeof(int));
  file.read(reinterpret_cast<char *>(&height), sizeof(int));

  std::cout << "Width: " << width << "\n";
  std::cout << "Height: " << height << "\n";

  cv::Mat imgHSV(height, width, CV_8UC3);

  for (int y = 0; y < height; ++y) {
    cv::Vec3b *row = imgHSV.ptr<cv::Vec3b>(y);

    for (int x = 0; x < width; ++x) {

      uint16_t packed;
      file.read(reinterpret_cast<char *>(&packed), sizeof(uint16_t));

      cv::Vec3b hsv = unpackHSV(packed);
      row[x] = hsv;
    }
  }

  file.close();

  cv::Mat imgBGR;
  cv::cvtColor(imgHSV, imgBGR, cv::COLOR_HSV2BGR);

  cv::imshow("Image", imgBGR);
  cv::waitKey(0);
}

void writeImageBinaryHSV(cv::Mat &imgBGR) {

  cv::Mat imgHSV;
  cv::cvtColor(imgBGR, imgHSV, cv::COLOR_BGR2HSV);

  int width = imgHSV.cols;
  int height = imgHSV.rows;

  std::ofstream file("bits.bin", std::ios::binary);

  file.write(reinterpret_cast<char *>(&width), sizeof(int));
  file.write(reinterpret_cast<char *>(&height), sizeof(int));

  for (int y = 0; y < height; ++y) {
    cv::Vec3b *row = imgHSV.ptr<cv::Vec3b>(y);

    for (int x = 0; x < width; ++x) {
      cv::Vec3b pixel = row[x];

      uchar h = pixel[0];
      uchar s = pixel[1];
      uchar v = pixel[2];

      uint16_t packed = packHSV(h, s, v);

      file.write(reinterpret_cast<char *>(&packed), sizeof(uint16_t));
    }
  }

  file.close();
}
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
