#include "socket.h"
#include <time.h>
#include <unistd.h>
#include "ssl.h"
#include "log.h"
#include <errno.h>
int main(void )
{
    socket_fd_t sock_fd = create_client_socket(SSL_PORT);
    if (sock_fd == -1) {
        close_socket(sock_fd);
        LOG_ERROR("sockfd create or connect failed.");
        exit(EXIT_FAILURE);
    }
    uint8_t buffer[MAX_BUFFER_SIZE];
    memset(buffer, 0, MAX_BUFFER_SIZE);
    while (1){
        time_t now;
        time(&now);

        char *str = "hello world.";
        ssl_header_t header;
        header.len = strlen(str) + 1;
        header.version = SSL_VERSION;
        header.type = SSL_TYPE;
        header.time =  (unsigned int)(now - BASE_TIME_STAMP);
        memcpy(buffer, &header, HEADER_LENGTH_BYTE);
        memcpy(&buffer[HEADER_LENGTH_BYTE], str, strlen(str) + 1);

        LOG_INFO("send data : len: %d  time %u", header.len, header.time);

        size_t ret = send_data(sock_fd, buffer, MAX_BUFFER_SIZE);
        if (ret == -1) {
            close_socket(sock_fd);

            LOG_ERROR("Send failed");
            exit(EXIT_FAILURE);
        }
        sleep(5);
    }
    close_socket(sock_fd);
    return 0;
    
}