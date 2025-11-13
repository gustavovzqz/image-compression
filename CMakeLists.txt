cmake_minimum_required(VERSION 4.0)

set(PROJECT_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR})

include_directories(${PROJECT_ROOT_DIR}) 

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic")

add_subdirectory(main)


