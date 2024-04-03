# test/CMakeLists.txt
cmake_minimum_required(VERSION 3.10)
project(test)

# 添加库
add_library(test test.c)
# 设置库的包含目录
target_include_directories(test PUBLIC .)

