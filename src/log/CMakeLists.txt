# log/CMakeLists.txt
cmake_minimum_required(VERSION 3.10)
project(log)
include_directories(include/)

# 添加库
add_library(log log.c)

# 设置库的包含目录
target_include_directories(log PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

