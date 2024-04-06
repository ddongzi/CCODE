# ssl/CMakeLists.txt
cmake_minimum_required(VERSION 3.10)
project(ssl)
# 添加源文件
file(GLOB SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/*.c)
# 添加头文件目录
include_directories(${HEAD_PATH}/ssl ${HEAD_PATH}/log)
# 生成库
add_library(ssl SHARED ${SOURCES})

# =============链接自己的库============
set(LIBS_PATH ${HOME}/library)
link_directories(${LIBS_PATH})
link_libraries(log)

# =================链接GNUTLS库================
# 设置 GnuTLS 路径
find_package(GnuTLS REQUIRED)
# 包含 GnuTLS 头文件
target_include_directories(ssl PRIVATE ${GNUTLS_INCLUDE_DIRS})
# 链接 GnuTLS 库到可执行文件
target_link_libraries(ssl gnutls)

