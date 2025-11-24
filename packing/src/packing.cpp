#include <cstdint>
#include <fstream>
#include <iostream>
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

bool closeEnough(uint16_t src, uint16_t dest) {

  cv::Vec3b src_vec = unpackHSV(src);
  cv::Vec3b dest_vec = unpackHSV(dest);

  uchar h_diff = abs(src_vec[0] - dest_vec[0]);
  uchar s_diff = abs(src_vec[1] - dest_vec[1]);
  uchar v_diff = abs(src_vec[2] - dest_vec[2]);

  bool a = (v_diff < 20 && s_diff < 20 && h_diff < 20);

  if (!a) {
    // std::cout << "Hd:" << (int)h_diff << " ";
    // std::cout << "Sd:" << (int)s_diff << " ";
    // std::cout << "Vd:" << (int)v_diff << "\n";
  }
  return a;
}

void writeImageBinaryHSV(cv::Mat &imgBGR) {
  cv::Mat imgHSV;
  cv::cvtColor(imgBGR, imgHSV, cv::COLOR_BGR2HSV);

  int width = imgHSV.cols;
  int height = imgHSV.rows;

  std::ofstream file("bits.bin", std::ios::binary);
  if (!file) {
    std::cerr << "Erro ao abrir arquivo para escrita.\n";
    return;
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

      if (closeEnough(packed, prevPacked) && runLength < 255) {
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

  file.write(reinterpret_cast<char *>(&prevPacked), sizeof(uint16_t));
  file.write(reinterpret_cast<char *>(&runLength), sizeof(uint8_t));

  file.close();
}

void readImageBinaryHSV() {
  std::ifstream file("bits.bin", std::ios::binary);
  if (!file) {
    std::cerr << "Erro ao abrir arquivo para leitura.\n";
    return;
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

  cv::imshow("Imagem Recuperada", imgBGR);
  cv::waitKey(0);
}
