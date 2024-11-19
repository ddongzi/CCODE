#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>

int main(int argc, char const *argv[])
{
    int fd[2];
    pid_t pid;
    char line[30];

    if (pipe(fd) < 0) 
        printf("e");
    if ((pid = fork()) < 0) {
        printf("E");
    } else if (pid > 0) {
        close(fd[0]);
        write(fd[1], "hello world\n", 12);
    } else {
        close(fd[1]); // 关闭管道 子进程写端
        read(fd[0], line, 12);
        puts(line);
    }
    return 0;
}
