#include "apue.h"
#include <fcntl.h>

#define	read_lock(fd, offset, whence, len) \
			lock_reg((fd), F_SETLK, F_RDLCK, (offset), (whence), (len))
#define	readw_lock(fd, offset, whence, len) \
			lock_reg((fd), F_SETLKW, F_RDLCK, (offset), (whence), (len))
#define	write_lock(fd, offset, whence, len) \
			lock_reg((fd), F_SETLK, F_WRLCK, (offset), (whence), (len))
#define	writew_lock(fd, offset, whence, len) \
			lock_reg((fd), F_SETLKW, F_WRLCK, (offset), (whence), (len))
#define	un_lock(fd, offset, whence, len) \
			lock_reg((fd), F_SETLK, F_UNLCK, (offset), (whence), (len))
int lock_reg(int fd, int cmd, int type, off_t offset, int whence, off_t len)
{
    struct flock lock;
    lock.l_type = type; /* F_RDLCK, F_WRLCK, F_UNLCK*/
    lock.l_start = offset;
    lock.l_whence = whence;
    lock.l_len = len; /*#bytes (o means to EOF)*/

    return fcntl(fd, cmd, &lock);
}
pid_t lock_test(int fd, int type, off_t offset, int whence, off_t len)
{
    struct flock flock;
    flock.l_type = type;
    flock.l_start = offset;
    flock.l_whence = whence;
    flock.l_len = len;

    if (fcntl(fd, F_GETFL, &flock) < 0) {
        err_sys("fcntl error");
    }
    if (flock.l_type == F_UNLCK) {
        return 0;
    }
    return flock.l_pid;
}