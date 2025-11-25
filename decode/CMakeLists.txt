project(decode) 

add_executable(decode decode.cpp)

target_link_libraries(decode
    opencv_core
    opencv_imgproc
    opencv_highgui
)

target_include_directories(decode PRIVATE ${OpenCV_INCLUDE_DIRS})
target_link_libraries(decode packing huffman)
