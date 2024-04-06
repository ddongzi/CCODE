cmake_minimum_required(VERSION 3.20)
project(ssl_test)



# =============链接自己的库============
# 添加头文件目录
include_directories(
        ${HEAD_PATH}/ssl
        ${HEAD_PATH}/log
)

set(LIBS_PATH ${HOME}/library)
link_directories(${LIBS_PATH})
link_libraries(log ssl)

add_executable(test_print_cipher_suite_list test_print_cipher_suite_list.c)


add_test(NAME test_print_cipher_suite_list
        COMMAND test_print_cipher_suite_list)

