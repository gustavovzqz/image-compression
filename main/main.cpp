#include <opencv2/opencv.hpp>
#include <iostream>
#include <vector>

int main() {
    // Load image
    cv::Mat img = cv::imread("image.png", cv::IMREAD_COLOR);
    if (img.empty()) {
        std::cerr << "Failed to open image\n";
        return 1;
    }

    int width = img.cols;
    int height = img.rows;

    // Create a byte matrix [height][width][3] using cv::Vec3b
    std::vector<std::vector<cv::Vec3b>> matrix(height, std::vector<cv::Vec3b>(width));

    for (int y = 0; y < height; ++y) {
        const cv::Vec3b* row = img.ptr<cv::Vec3b>(y);
        for (int x = 0; x < width; ++x) {
            matrix[y][x] = row[x]; // BGR
        }
    }

    // Display the image
    cv::imshow("Image", img);
    cv::waitKey(0);
    return 0;
}
