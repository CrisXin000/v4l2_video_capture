cmake_minimum_required(VERSION 3.17)
project(video_capture)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_BUILD_TYPE DEBUG)

find_package(OpenCV REQUIRED)

include_directories(${OpenCV_INCLUDE_DIRS})

add_executable(video_capture main.cpp)
target_link_libraries(video_capture ${OpenCV_LIBS})

add_executable(multicap singleWindowMultiImage.cpp)
target_link_libraries(multicap ${OpenCV_LIBS})