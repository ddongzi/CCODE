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
    while (1){
        time_t now;
        time(&now);

        char *str = "hello world . ";
        ssl_header_t header;
        header.len = strlen(str) + 1;
        header.version = SSL_VERSION;
        header.type = SSL_TYPE;
        header.time =  now;
        int position = 0;
        LOG_INFO("Create header : len: %d  time %lu", header.len, header.time);
        memcpy(buffer, &header, sizeof(header));
        LOG_INFO("Create header into buffer succuess.");
        position = sizeof(header);
        sprintf(buffer + position, str);
        LOG_INFO("Create body extrainfo into buffer succuess.");

        size_t ret = send_data(sock_fd, buffer, MAX_BUFFER_SIZE);
        if (ret == -1) {
            close_socket(sock_fd);

            LOG_ERROR("Send failed");
            exit(EXIT_FAILURE);
        }
        sleep(2);
    }
    close_socket(sock_fd);
    return 0;
    
}