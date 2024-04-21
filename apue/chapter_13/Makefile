# 定义编译器
CC = gcc
# 定义编译参数
CFLAGS = -Wall -Wextra -g

# 定义目标文件
TARGET = myprogram
# 定义源文件
SRCS = main.c utils.c
# 定义目标文件
OBJS = $(SRCS:.c=.o)

# 默认构建目标
all: $(TARGET)

# 构建目标文件
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

# 生成目标文件
%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

# 清理生成的目标文件
clean:
	rm -f $(OBJS) $(TARGET)

