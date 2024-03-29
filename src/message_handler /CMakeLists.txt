# message_handle/CMakeLists.txt
cmake_minimum_required(VERSION 3.10)
project(message_handler)
# 添加库
add_library(message_handler message_handler.c)

# 设置库的包含目录
target_include_directories(message_handler PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
