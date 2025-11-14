project(main) 


add_executable(main main.cpp)

target_link_libraries(main
    opencv_core
    opencv_imgproc
    opencv_highgui
)

target_include_directories(main PRIVATE ${OpenCV_INCLUDE_DIRS})

# target_link_libraries(main scheduler parser)
