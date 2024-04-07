# server/CMakeLists.txt
cmake_minimum_required(VERSION 3.20)
project(server)

include_directories(${HEAD_PATH}/server 
    ${HEAD_PATH}/network 
    ${HEAD_PATH}/log
    ${HEAD_PATH}/ssl
)

set(LIBS_PATH ${HOME}/library)
# 设置库搜索路径 ,自己开发的库 需要设置目录
link_directories(${LIBS_PATH})
link_libraries(log ssl network)
# add_executable(server main.c)
add_executable(ssl_server ssl_server.c)

add_definitions(-DSERVER_DEBUG)

# =================链接GNUTLS库================
link_directories(${GNUTLS_PATH})
# 包含 GnuTLS 头文件
include_directories(${GNUTLS_HEAD_PATH})
# 链接 GnuTLS 库到可执行文件
link_libraries(${GNUTLS_LIB_PATH})


