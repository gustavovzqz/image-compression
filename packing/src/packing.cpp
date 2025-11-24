#include "packing/include/packing.hpp"
#include <fstream>
#include <opencv2/opencv.hpp>
#include <stdexcept>

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

bool closeEnough(uint16_t src, uint16_t dest, uint8_t H_MAX_DIST,
                 uint8_t S_MAX_DIST, uint8_t V_MAX_DIST) {

  cv::Vec3b src_vec = unpackHSV(src);
  cv::Vec3b dest_vec = unpackHSV(dest);

  uchar h_diff = abs(src_vec[0] - dest_vec[0]);
  uchar s_diff = abs(src_vec[1] - dest_vec[1]);
  uchar v_diff = abs(src_vec[2] - dest_vec[2]);

  return (v_diff < V_MAX_DIST && s_diff < S_MAX_DIST && h_diff < H_MAX_DIST);
}

void compressBRGImage(cv::Mat &imgBGR, std::string path, uint8_t H_MAX_DIST,
                      uint8_t S_MAX_DIST, uint8_t V_MAX_DIST) {
  cv::Mat imgHSV;
  cv::cvtColor(imgBGR, imgHSV, cv::COLOR_BGR2HSV);

  int width = imgHSV.cols;
  int height = imgHSV.rows;

  std::ofstream file(path, std::ios::binary);
  if (!file) {
    throw std::invalid_argument("path is invalid");
  }

  file.write(reinterpret_cast<char *>(&width), sizeof(int));
  file.write(reinterpret_cast<char *>(&height), sizeof(int));

  uint16_t prevPacked =
      packHSV(imgHSV.at<cv::Vec3b>(0, 0)[0], imgHSV.at<cv::Vec3b>(0, 0)[1],
              imgHSV.at<cv::Vec3b>(0, 0)[2]);

  // começar com 0 pq vai repetir 100% na primeira
  // e precisa ser 16 bits pq quando chegar em 256 não vai zerar
  uint16_t runLength = 0;

  std::vector<uint16_t> packedValues;

  uint32_t h_sum = 0;
  uint32_t s_sum = 0;
  uint32_t v_sum = 0;

  for (int y = 0; y < height; ++y) {
    cv::Vec3b *row = imgHSV.ptr<cv::Vec3b>(y);

    for (int x = 0; x < width; ++x) {
      uchar h = row[x][0];
      uchar s = row[x][1];
      uchar v = row[x][2];

      uint16_t packed = packHSV(h, s, v);

      if (closeEnough(packed, prevPacked, H_MAX_DIST, S_MAX_DIST, V_MAX_DIST) &&
          runLength < 255) {
        runLength++;
      } else {
        uint16_t packedMean =
            packHSV(h_sum / runLength, s_sum / runLength, v_sum / runLength);
        file.write(reinterpret_cast<char *>(&packedMean), sizeof(uint16_t));
        file.write(reinterpret_cast<char *>(&runLength), sizeof(uint8_t));
        prevPacked = packed;
        h_sum = s_sum = v_sum = 0;
        runLength = 1;
      }
      h_sum += h;
      s_sum += s;
      v_sum += v;
    }
  }

  uint16_t packedMean =
      packHSV(h_sum / runLength, s_sum / runLength, v_sum / runLength);

  file.write(reinterpret_cast<char *>(&packedMean), sizeof(uint16_t));
  file.write(reinterpret_cast<char *>(&runLength), sizeof(uint8_t));

  file.close();
}

cv::Mat uncompressBinary(std::string path) {
  std::ifstream file(path, std::ios::binary);
  if (!file) {
    throw std::invalid_argument("path is invalid");
  }

  int width, height;
  file.read(reinterpret_cast<char *>(&width), sizeof(int));
  file.read(reinterpret_cast<char *>(&height), sizeof(int));

  cv::Mat imgHSV(height, width, CV_8UC3);

  int totalPixels = width * height;
  int idx = 0;

  while (idx < totalPixels && file) {
    uint16_t packed;
    uint8_t runLength;

    file.read(reinterpret_cast<char *>(&packed), sizeof(uint16_t));
    file.read(reinterpret_cast<char *>(&runLength), sizeof(uint8_t));

    for (int i = 0; i < runLength && idx < totalPixels; ++i, ++idx) {
      int y = idx / width;
      int x = idx % width;
      imgHSV.at<cv::Vec3b>(y, x) = unpackHSV(packed);
    }
  }

  file.close();

  cv::Mat imgBGR;
  cv::cvtColor(imgHSV, imgBGR, cv::COLOR_HSV2BGR);
  return imgBGR;
}
