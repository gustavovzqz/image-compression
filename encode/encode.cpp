#include "huffman/include/huffman.hpp"
#include "packing/include/packing.hpp"
#include <iostream>
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/opencv.hpp>

uint8_t MAX_DIST_H, MAX_DIST_S, MAX_DIST_V;

int main(int argc, char *argv[]) {
  if (argc != 4) {
    std::cerr << "Uso: " << argv[0]
              << " <imagem_entrada> <arquivo_saida> <max_dist>\n";
    return 1;
  }

  std::string inputImage = argv[1];
  std::string outputFile = argv[2];
  int dist = std::stoi(argv[3]);

  if (dist < 0 || dist > 255) {
    std::cerr << "max_dist deve estar entre 0 e 255\n";
    return 1;
  }

  MAX_DIST_H = MAX_DIST_S = MAX_DIST_V = static_cast<uint8_t>(dist);

  cv::Mat bgr_img = cv::imread(inputImage, cv::IMREAD_COLOR);
  if (bgr_img.empty()) {
    std::cerr << "Falha ao abrir a imagem\n";
    return 1;
  }

  compressBRGImage(bgr_img, "rle.bin", MAX_DIST_H, MAX_DIST_S, MAX_DIST_V);
  runHuffman(outputFile);
  return 0;
}
