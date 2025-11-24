#ifndef PACKING_INCLUDE_PACKING_HPP
#define PACKING_INCLUDE_PACKING_HPP

#include <opencv2/core.hpp>

cv::Mat uncompressBinary(std::string path);
void compressBRGImage(cv::Mat &brg_img, std::string path, uint8_t MAX_DIST_H,
                      uint8_t MAX_DIST_S, uint8_t MAX_DIST_V);

#endif
