cmake_minimum_required(VERSION 2.8.3)
project(camera_opencv)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
# set(CMAKE_BUILD_TYPE "Debug")

find_package( OpenCV REQUIRED )
include_directories( ${OpenCV_INCLUDE_DIRS} )

# set(V4L2_LIBRARIES v4l2)
# include_directories(include ${EIGEN3_INCLUDE_DIR} ${YAML_CPP_INCLUDE_DIR})

add_executable(camera_opencv src/camera_opencv.cpp)
# add_executable(camera_opencv src/test.cpp)
# add_executable(camera_opencv src/camera_opencv.c)

# target_link_libraries(camera_opencv ${OpenCV_LIBS})
target_link_libraries(camera_opencv ${OpenCV_LIBS})
