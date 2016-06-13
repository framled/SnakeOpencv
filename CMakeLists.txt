cmake_minimum_required(VERSION 3.2)
project(SnakeOpenCV)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES
    src/MainSnake.cpp)

find_package(PCL 1.2 REQUIRED)
find_package( OpenCV REQUIRED )
find_package(Qt5Widgets)

include_directories(${PCL_INCLUDE_DIRS})
link_directories(${PCL_LIBRARY_DIRS})
add_definitions(${PCL_DEFINITIONS})

add_executable(SnakeOpenCV ${SOURCE_FILES})
target_link_libraries (SnakeOpenCV ${OpenCV_LIBS} ${PCL_LIBRARIES} Qt5::Widgets)