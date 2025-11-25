#include "huffman/include/huffman.hpp"
#include "packing/include/packing.hpp"
#include <filesystem>
#include <iostream>
#include <opencv2/opencv.hpp>

namespace fs = std::filesystem;

fs::path TEMP_FILE = "temp.bin";

int main(int argc, char *argv[]) {
  if (argc != 3) {
    std::cerr << "Uso: " << argv[0] << " <imagem_comprimida> <saida.bmp>\n";
    return 1;
  }

  std::string inputCompressed = argv[1];
  std::string outputBmp = argv[2];

  reconstructOriginal(inputCompressed, TEMP_FILE);

  cv::Mat output_img = uncompressBinary(TEMP_FILE);
  fs::remove(TEMP_FILE);

  if (output_img.empty()) {
    std::cerr << "Erro: imagem reconstruída vazia\n";
    return 1;
  }

  cv::imwrite(outputBmp, output_img);

  return 0;
}
