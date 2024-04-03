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
target_link_libraries(log PRIVATE )

# ===============其他模块=========================
# 添加test模块
link_directories(../../src/test)  # 在哪里查找库文件
target_link_libraries(log PRIVATE ../../src/test)
target_include_directories(log PRIVATE ../../src/test)
