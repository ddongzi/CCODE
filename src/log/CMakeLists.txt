# log/CMakeLists.txt
cmake_minimum_required(VERSION 3.10)
project(log)

# =============测试================
# 源目录
add_executable(log_test tests/log_test.c)
add_test(NAME log_test_name COMMAND log_test)


# =============log库设置===========
# 添加库
add_library(log log.c)
# 设置库的包含目录
target_include_directories(log PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})



# ===============其他模块=========================

# 添加test模块
link_directories(/test)  # 在哪里查找库文件
target_link_libraries(log PRIVATE test)
target_include_directories(log PRIVATE /test)
