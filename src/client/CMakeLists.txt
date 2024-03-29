# network/CMakeLists.txt
cmake_minimum_required(VERSION 3.10)
project(client)
# 添加库
add_library(client main.c)

# 设置库的包含目录
target_include_directories(client PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
