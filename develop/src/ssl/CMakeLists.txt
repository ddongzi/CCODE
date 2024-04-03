cmake_minimum_required(VERSION 3.20)
project(ssl)

include_directories(${HEAD_PATH}/server ${HEAD_PATH}/network ${HEAD_PATH}/ssl)

set(LIBS_PATH ${HOME}/library)
# 设置库搜索路径 ,自己开发的库 需要设置目录
link_directories(${LIBS_PATH})
link_libraries(log)
add_executable(ssl main.c)


add_definitions(-DSSL_DEBUG)




# =================链接GNUTLS库================
# 设置 GnuTLS 路径
find_package(GnuTLS REQUIRED)
# 包含 GnuTLS 头文件
target_include_directories(ssl PRIVATE ${GNUTLS_INCLUDE_DIRS})
# 链接 GnuTLS 库到可执行文件
target_link_libraries(ssl gnutls)

