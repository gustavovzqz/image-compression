#include <fstream>
#include <iostream>
#include <opencv2/core.hpp>
#include <opencv2/opencv.hpp>

uint16_t packHSV(uchar h, uchar s, uchar v) {
  uint16_t H6 = (uint16_t)(h * 63 / 179);
  uint16_t S4 = (uint16_t)(s >> 4);
  uint16_t V6 = (uint16_t)(v >> 2);
  return (H6 << 10) | (S4 << 6) | V6;
}

cv::Vec3b unpackHSV(uint16_t packed) {
  uint16_t H6 = (packed >> 10) & 0x3F;
  uint16_t S4 = (packed >> 6) & 0x0F;
  uint16_t V6 = packed & 0x3F;
  uchar h = (uchar)(H6 * 179 / 63);
  uchar s = (uchar)(S4 << 4);
  uchar v = (uchar)(V6 << 2);
  return cv::Vec3b(h, s, v);
}

void writeImageBinaryHSV(cv::Mat &imgBGR) {
  cv::Mat imgHSV;
  cv::cvtColor(imgBGR, imgHSV, cv::COLOR_BGR2HSV);

  int width = imgHSV.cols;
  int height = imgHSV.rows;

  std::ofstream file("bits.bin", std::ios::binary);
  if (!file)
    return;

  file.write(reinterpret_cast<char *>(&width), sizeof(int));
  file.write(reinterpret_cast<char *>(&height), sizeof(int));

  for (int y = 0; y < height; ++y) {
    cv::Vec3b *row = imgHSV.ptr<cv::Vec3b>(y);
    uint16_t prevPacked = packHSV(row[0][0], row[0][1], row[0][2]);
    uint16_t runLength = 1;

    for (int x = 1; x <= width; ++x) {
      uint16_t packed =
          (x < width) ? packHSV(row[x][0], row[x][1], row[x][2]) : 0xFFFF;

      if (packed == prevPacked && runLength < 255) {
        runLength++;
      } else {
        file.write(reinterpret_cast<char *>(&prevPacked), sizeof(uint16_t));
        file.write(reinterpret_cast<char *>(&runLength), sizeof(uint8_t));
        runLength = 1;
        prevPacked = packed;
      }
    }
  }

  file.close();
}

void readImageBinaryHSV() {
  std::ifstream file("bits.bin", std::ios::binary);
  if (!file)
    return;

  int width, height;
  file.read(reinterpret_cast<char *>(&width), sizeof(int));
  file.read(reinterpret_cast<char *>(&height), sizeof(int));

  cv::Mat imgHSV(height, width, CV_8UC3);
  int x = 0, y = 0;

  while (file.peek() != EOF && y < height) {
    uint16_t packed;
    uint8_t runLength;
    file.read(reinterpret_cast<char *>(&packed), sizeof(uint16_t));
    file.read(reinterpret_cast<char *>(&runLength), sizeof(uint8_t));

    for (int i = 0; i < runLength; ++i) {
      imgHSV.at<cv::Vec3b>(y, x) = unpackHSV(packed);
      x++;
      if (x >= width) {
        x = 0;
        y++;
      }
    }
  }

  file.close();

  cv::Mat imgBGR;
  cv::cvtColor(imgHSV, imgBGR, cv::COLOR_HSV2BGR);

  cv::imshow("Image", imgBGR);
  cv::waitKey(0);
}
