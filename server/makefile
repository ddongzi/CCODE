# 目标名称
TARGET = tcpserver tcpclient

# 目标文件目录
BUILD_DIR = build
OBJ_SERVER = $(addprefix $(BUILD_DIR)/, array.o connect_manager.o connection_table.o connection.o \
	log.o message_manager.o message_queue.o message.o \
	queue.o server.o socket.o thread.o threadpool.o )
OBJ_CLIENT = $(addprefix $(BUILD_DIR)/, client.o message.o log.o)

# 源文件
SRC_SERVER = array.c connect_manager.c connection_table.c connection.c \
	log.c message_manager.c message_queue.c message.c \
	queue.c server.c socket.c thread.c threadpool.c 
SRC_CLIENT = client.c

# 编译器
CC = gcc

# 编译选项
CFLAGS = -Wall -Wextra -g
CFLAGS += -DDEBUG

# 默认目标
all: $(TARGET)

# 生成 tcpserver
tcpserver: $(OBJ_SERVER)
	$(CC) $(CFLAGS) $(OBJ_SERVER) -o $@

# 生成 tcpclient
tcpclient: $(OBJ_CLIENT)
	$(CC) $(CFLAGS) $(OBJ_CLIENT) -o $@

# 编译规则
$(BUILD_DIR)/%.o: %.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# 确保 build/ 目录存在
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# 清理目标
clean:
	rm -rf $(TARGET) $(BUILD_DIR)
