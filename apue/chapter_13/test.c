#include <syslog.h>
#include <unistd.h>
#include <stdio.h>
int main(void)
{
    // 设置新的根目录为 /new_root
    if (chroot("/home/dong") != 0) {
        perror("chroot");
        return 1;
    }

    // 调用 openlog 设置日志标识符为 example
    openlog("example", LOG_PID, LOG_USER);

    // 执行根目录已经被改变的操作
    // 在这里，当前进程的根目录已经被改变为 /new_root
    // 任何文件系统操作都将在 /new_root 及其子目录下执行

    // 写入日志
    syslog(LOG_INFO, "This is a test message.");

    closelog();

    return 0;
}
