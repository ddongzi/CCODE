// includes
#include "socket.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <signal.h>
#include <assert.h>
#include <sys/select.h>
#include <fcntl.h>
#include "log.h"
#include <string.h>
#include <errno.h>

// defines
#define SOCKADDR_LEN (sizeof(struct sockaddr))
#define MAX(a,b) ((a) > (b) ? (a) : (b))

// internal property
static fd_set fds_read_base;    // 
static fd_set fds_write_base;
static fd_set fds_read_ready;  // select调用返回后fdset
static fd_set fds_write_ready;

static int maxfd;   // select 参数
static int maxfd_read;
static int maxfd_write;


// internal helper function
static int create_tcp_socket()
{
    int tcp_socket;
    if ((tcp_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0) 
        perror("create tcp socket error!");
    return tcp_socket;
}
/**
 * @brief 构造server socket套接字地址控制结构 
 * 
 * @param [in] sscs 
 * @param [in] port 
 */
static void prepare_server_socket(struct sockaddr_in*sscs, in_port_t port)
{
    sscs->sin_addr.s_addr = INADDR_ANY;
    sscs->sin_family = AF_INET;
    sscs->sin_port = htons(port);
}
static void bind_server_socket(int server_socket, in_port_t port)
{
    struct sockaddr_in sscs;
    prepare_server_socket(&sscs, port);
    if (bind(server_socket, (struct sockaddr*)&sscs, SOCKADDR_LEN) < 0) {
        perror("bind port failed");
    }
}
static void start_listening(int server_socket)
{
    if (listen(server_socket, 5) < 0) {
        perror("listen error.");
    }
}


// public interface
/**
 * @brief 设置为非阻塞IO。
 *  1. write相关：如果套接字写缓冲区已满，阻塞IO会阻塞。非阻塞返回EWOULDBLOCK错误
 *  2. read相关：如果套接字读缓冲区为空，阻塞式会阻塞。 非阻塞返回EWOULDBLOCK错误
 *  3. accept：如果没有新连接到达，阻塞式会阻塞。 非阻塞返回EWOULDBLOCK错误
 *  4. connect:**还不明确
 * 
 * @param socket 
 * @param nonblocking 
 */
void socket_set_nonblocking(int socket, bool nonblocking)
{
    int flags = fcntl(socket, F_GETFL, 0);
    if (nonblocking) flags |= O_NONBLOCK;
    else flags &= O_NONBLOCK;
    fcntl(socket, F_SETFL, flags);
}
/**
 * @brief 把socket加入select read fdset
 *  初始化~fdset read base
 * @param [in] socket 
 */
void socket_add_socket_for_reading(int socket)
{
    LOG_DEBUG("starting to watch a socket#%d for read events", socket);
    maxfd_read = MAX(maxfd_read, socket);
    maxfd = MAX(maxfd_read, maxfd);
    FD_SET(socket, &fds_read_base);
    LOG_DEBUG("max_fd now: %d", maxfd);
}
void socket_add_socket_for_writing(int socket)
{
    LOG_DEBUG("starting to watch a socket for write events");
    maxfd_write = MAX(maxfd_write, socket);
    maxfd = MAX(maxfd_write, maxfd);
    FD_SET(socket, &fds_write_base);
    LOG_DEBUG("max_fd now: %d", maxfd);
}
void socket_remove_socket_for_reading(int socket)
{
    if (socket == maxfd_read) {
        maxfd_read--;
        maxfd = MAX(maxfd_read, maxfd_write);
    }
    FD_CLR(socket, &fds_read_base);
    FD_CLR(socket, &fds_read_ready);
}
void socket_remove_socket_for_writing(int socket)
{
    if (socket == maxfd_write) {
        maxfd_write--;
        maxfd = MAX(maxfd_read, maxfd_write);
    }
    FD_CLR(socket, &fds_write_base);
    FD_CLR(socket, &fds_write_ready);
}

bool socket_connect(int client_socket, in_addr_t server_ip, in_port_t server_port)
{
    struct sockaddr_in sin;
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = server_ip;
    sin.sin_port = server_port;

    return (connect(client_socket, (struct sockaddr*)&sin, sizeof(sin)) == 0);
}
int socket_create_client_socket()
{
    return create_tcp_socket();
}
int socket_create_server_socket(in_port_t port)
{
    int server_socket = create_tcp_socket();

    bind_server_socket(server_socket, port);
    start_listening(server_socket);
    LOG_DEBUG("create server socket %d", server_socket);
    return server_socket;
}
void socket_init()
{
    // 允许向一个已经关闭的套接字写，否则会产生SIGPIPE信号，默认中断程序
    signal(SIGPIPE, SIG_IGN);
}
void socket_ipaddr_to_str(in_addr_t ip, char* str)
{
    assert(str);
    char* p = (char *)&ip;
    sprintf(str, "%u.%u.%u.%u", p[0], p[1], p[2], p[3]);
}
/**
 * @brief 检查socket是否ready读，从fdset中
 * 
 * @param [in] socket 
 * @return true 
 * @return false 
 */
bool socket_is_ready_for_reading(int socket)
{
    return (FD_ISSET(socket, &fds_read_ready));
}
bool socket_is_ready_for_writing(int socket)
{
    return (FD_ISSET(socket, &fds_write_ready));
}
int socket_recv(int socket, char* buffer, size_t length)
{
    assert(buffer);
    return recv(socket, buffer, length, 0);
}
int socket_send(int socket, char* buffer, size_t length)
{
    assert(buffer);
    return send(socket, buffer, length, 0);
}

// UNIX域套接字
void socket_create_socketpair(int* sender, int* receiver)
{
    int fd[2];

    // ipc local 
    if (socketpair(AF_LOCAL, SOCK_STREAM, 0, fd) < 0) {
        perror(" socket pair error");
    }
    *sender = fd[0];    // 管道读端
    *receiver = fd[1];  // 管道写端
    LOG_DEBUG("create ipc socket, sender %d, receiver %d", fd[0], fd[1]);
}
/**
 * @brief 等待接受一个新connect
 * 
 * @param [in] server_socket 
 * @param [out] client_ipaddr 
 * @param [out] client_port 
 * @return int ：work socket. error returns -1.
 */
int socket_create_worker_socket(int server_socket, in_addr_t* client_ipaddr, in_port_t* client_port)
{
// TODO ? nonblocking?

    int worker_socket;
    struct sockaddr_in sa;
    socklen_t sockaddr_len = SOCKADDR_LEN;

    worker_socket = accept(server_socket, (struct sockaddr*)&sa, &sockaddr_len);

    if (worker_socket == -1) {
        LOG_ERROR("accept failed. ERRNO:%s", strerror(errno));
        return -1;
    }
    if (client_ipaddr)
        *client_ipaddr = sa.sin_addr.s_addr;
    if (client_port)
        *client_port = htons(sa.sin_port);
    LOG_DEBUG("Create worker socket %d.", worker_socket);
    return worker_socket;
}

/**
 * @brief 调用select等待socket ready
 * 
 * @return int : 返回ready的 fd数目. 即ready的事件数
 */
int socket_wait_events()
{
    int ret;
    for(;;) {
        fds_read_ready = fds_read_base;
        fds_write_ready = fds_write_base;
        ret = select(maxfd + 1, &fds_read_ready, &fds_write_ready, NULL, NULL);
        if (ret == -1) {
            perror("select ret -1. Maybe a signal was caught");
        }
        if (ret == 0) {
            perror("there is no ready socket for write/read.");
        }
        break;
    }
    return ret;
}
int socket_close(int socket)
{
    return close(socket);
}





