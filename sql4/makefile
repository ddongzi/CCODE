# Makefile

# 目标名称
TARGET = db

# 源文件
SRC = main.c

# 编译器
CC = gcc

# 编译选项
CFLAGS = -DDEBUG -g

# 默认目标
all: $(TARGET)

# 目标规则
$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET)

# 清理目标
clean:
	rm -f $(TARGET)
