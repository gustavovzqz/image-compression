#include "huffman/include/huffman.hpp"
#include "packing/include/packing.hpp"
#include <filesystem>
#include <iostream>
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/opencv.hpp>
namespace fs = std::filesystem;

fs::path TEMP_FILE = "temp.bin";

int main(int argc, char *argv[]) {
  if (argc != 2) {
    std::cerr << "Uso: " << argv[0] << " <imagem_comprimida> \n";
    return 1;
  }

  std::string inputImage = argv[1];

  reconstructOriginal(inputImage, TEMP_FILE);

  cv::Mat output_img = uncompressBinary(TEMP_FILE);
  fs::remove(TEMP_FILE);
  cv::imshow("Imagem: ", output_img);
  cv::waitKey(0);

  return 0;
}
