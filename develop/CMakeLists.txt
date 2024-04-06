cmake_minimum_required(VERSION 3.10)
project(MY
        VERSION "24.1"
        DESCRIPTION ""
        LANGUAGES "C"
)
# 触发断点调试
set (CMAKE_BUILD_TYPE Debug)

# 指定使用C11编译
set(CMAKE_C_STANDARD 11)
# 设置exe输出目录
set(HOME /home/dong/ALLCODE/develop)
set(EXECUTABLE_OUTPUT_PATH ${HOME}/bin)
# 设置库文件目录
set(LIBRARY_OUTPUT_PATH ${HOME}/library)
set(HEAD_PATH ${HOME}/include)

# 头文件位置
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
# 搜素源文件
file(GLOB SRC_LIST ${CMAKE_CURRENT_SOURCE_DIR}/*.c)
add_executable(main ${SRC_LIST})

# 添加子目录
add_subdirectory(src)
add_subdirectory(libs)

# 设置ctest开关
option(BUILD_TEST "Whether or not to build the tests" ON)
if (${BUILD_TEST})
    message(STATUS "Building tests")
    enable_testing()  # Enable testing only works in root scope
    add_subdirectory(tests)
endif ()
