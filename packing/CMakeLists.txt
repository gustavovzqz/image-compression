project(packing) 

set(HEADERS
  "include/packing.hpp"
)

SET(SOURCES
  "src/packing.cpp"
)

add_library(packing ${SOURCES} ${HEADERS})


target_link_libraries(packing
    opencv_core
    opencv_imgproc
    opencv_highgui
)
