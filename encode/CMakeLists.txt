project(encode) 

add_executable(encode encode.cpp)

target_link_libraries(encode
    opencv_core
    opencv_imgproc
    opencv_highgui
)

target_include_directories(encode PRIVATE ${OpenCV_INCLUDE_DIRS})
target_link_libraries(encode packing huffman)
