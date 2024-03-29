cmake_minimum_required(VERSION 3.10)
project(MY)

# 添加子目录
add_subdirectory(src)

# 添加可执行文件，并链接子目录中的库
add_executable(main main.c)
target_link_libraries(main PRIVATE network message_handler log)

# 设置C编译选项
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra")

# 打印详细的编译信息
set(CMAKE_VERBOSE_MAKEFILE on)
