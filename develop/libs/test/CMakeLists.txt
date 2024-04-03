# test/CMakeLists.txt
cmake_minimum_required(VERSION 3.10)
project(test)
# 添加源文件
file(GLOB SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/*.c)
# 添加头文件目录
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
# 生成库
add_library(test SHARED ${SOURCES})
