# network/CMakeLists.txt
cmake_minimum_required(VERSION 3.10)
project(network)

# 设置库的包含目录
include_directories(include/)

# 添加库
add_library(network network.c base_socket.c)

# 配置链接network库
link_directories(../log)  # 在哪里查找库文件
target_link_libraries(network PRIVATE log)
target_include_directories(network PRIVATE ../log/include)