# client/CMakeLists.txt
cmake_minimum_required(VERSION 3.20)
project(client)

include_directories(${HEAD_PATH}/client)

set(LIBS_PATH ${HOME}/library)
# 设置库搜索路径 ,自己开发的库 需要设置目录
link_directories(${LIBS_PATH})
link_libraries(log)
add_executable(client main.c)


add_definitions(-DCLIENT_DEBUG)



