#include <fstream>
#include <iostream>
#include <opencv2/core.hpp>
#include <opencv2/opencv.hpp>

void readImageBinary() {
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

  cv::Mat img(height, width, CV_8UC3);

  for (int y = 0; y < height; ++y) {
    cv::Vec3b *row = img.ptr<cv::Vec3b>(y);
    for (int x = 0; x < width; ++x) {
      uchar b, g, r;
      file.read(reinterpret_cast<char *>(&b), sizeof(uchar));
      file.read(reinterpret_cast<char *>(&g), sizeof(uchar));
      file.read(reinterpret_cast<char *>(&r), sizeof(uchar));

      row[x] = cv::Vec3b(b, g, r);
    }
  }

  file.close();

  cv::imshow("Image", img);
  cv::waitKey(0);
}

void writeImageBinary(cv::Mat &img) {

  int width = img.cols;
  int height = img.rows;

  std::ofstream file("bits.bin", std::ios::binary);

  file.write(reinterpret_cast<char *>(&width), sizeof(int));
  file.write(reinterpret_cast<char *>(&height), sizeof(int));

  // now writing every pixel in order

  for (int y = 0; y < img.rows; ++y) {
    cv::Vec3b *row = img.ptr<cv::Vec3b>(y);

    for (int x = 0; x < img.cols; ++x) {
      cv::Vec3b pixel = row[x];

      file.write(reinterpret_cast<char *>(&pixel[0]), sizeof(uchar));
      file.write(reinterpret_cast<char *>(&pixel[1]), sizeof(uchar));
      file.write(reinterpret_cast<char *>(&pixel[2]), sizeof(uchar));
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

  writeImageBinary(bgr_img);
  readImageBinary();

  // cv::Mat hls_image;
  // cv::cvtColor(bgr_img, hls_image, cv::COLOR_BGR2HLS);

  // cv::Mat img;
  // cv::cvtColor(hls_image, img, cv::COLOR_HLS2BGR);

  // cv::imshow("Image", bgr_img);
  // cv::waitKey(0);
  return 0;
}
