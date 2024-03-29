# network/CMakeLists.txt
cmake_minimum_required(VERSION 3.10)
project(server)
# 添加库
add_library(server ssl_server.c)

# 设置库的包含目录
target_include_directories(server PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# 配置链接network库
link_directories(../network)  # 在哪里查找库文件
target_link_libraries(server PRIVATE network)
target_include_directories(server PRIVATE ../network/include)

# 配置链接ssl库
link_directories(../ssl)  # 在哪里查找库文件
target_link_libraries(server PRIVATE ssl)
target_include_directories(server PRIVATE ../ssl/include)

# 配置链接log库
link_directories(../log)  # 在哪里查找库文件
target_link_libraries(server PRIVATE log)
target_include_directories(server PRIVATE ../log/include)