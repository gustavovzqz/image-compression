project(huffman) 

set(HEADERS
  "include/huffman.hpp"
)

SET(SOURCES
  "src/huffman.cpp"
)

add_library(huffman ${SOURCES} ${HEADERS})

target_link_libraries(huffman packing)


