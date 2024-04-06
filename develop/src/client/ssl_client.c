#include "socket.h"
#include <time.h>
#include <unistd.h>
#include "ssl.h"
#include "log.h"
#include <errno.h>
#include "gnutls/gnutls.h"

void my_handle_ssl(socket_fd_t sock_fd)
{
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
}
void handle_ssl(socket_fd_t sock_fd)
{
    gnutls_global_init();
    gnutls_session_t session;
    gnutls_credentials_type_t x_serv_cred;
    gnutls_init(&session, GNUTLS_CLIENT);
    gnutls_transport_set_int(session, sock_fd);

    int ret;
    const char *suites = "NONE:+VERS-TLS1.0:+RSA:+ARCFOUR-128:+AES-128-CBC:+AES-256-CBC:+SHA1:+MD5";
    const char *err;
    ret = gnutls_priority_set_direct(session, suites, &err);
    if (ret < 0) {
        fprintf(stderr, "Syntax error at: %s\n", err);
    } else {
        LOG_INFO("SET suites success.");
    }
    print_cipher_suite_list(suites);

    gnutls_protocol_t protocol = gnutls_protocol_get_version(session);
    LOG_INFO("CLIENT ssl version : %s", gnutls_protocol_get_name(protocol));

    gnutls_session_set_verify_cert(session, NULL, 0);

    ret = gnutls_handshake(session);
    if (ret < 0) {
        fprintf(stderr, "client Handshake failed : %s \n", gnutls_strerror(ret));
        close_socket(sock_fd);
        gnutls_deinit(session);
        gnutls_global_deinit();
        return;
    }
    // 关闭连接
    gnutls_bye(session, GNUTLS_SHUT_RDWR);
    close_socket(sock_fd);
    gnutls_deinit(session);
    gnutls_global_deinit();
}
int main(void )
{
    socket_fd_t sock_fd = create_client_socket(SSL_PORT);
    if (sock_fd == -1) {
        close_socket(sock_fd);
        LOG_ERROR("sockfd create or connect failed.");
        exit(EXIT_FAILURE);
    }
    handle_ssl(sock_fd);

    return 0;
    
}