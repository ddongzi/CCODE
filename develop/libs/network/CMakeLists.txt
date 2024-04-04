# network/CMakeLists.txt
cmake_minimum_required(VERSION 3.10)
project(network)
# 添加源文件
file(GLOB SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/*.c)
# 添加头文件目录
include_directories(${HEAD_PATH}/network ${HEAD_PATH}/log)

# 生成库
add_library(network SHARED ${SOURCES})


link_directories(${LIBS_PATH})
link_libraries(log)

