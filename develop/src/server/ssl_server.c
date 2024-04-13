//
// Created by dong on 3/29/24.
//

#include "server.h"
#include "socket.h"
#include "protocol_type.h"
#include "network_config.h"
#include "log.h"
#include "ssl.h"
#include <pthread.h>
#include <errno.h>
#include "gnutls/gnutls.h"

// 证书结构
gnutls_certificate_credentials_t x509_cred;

void print_ssl(ssl_header_t *header, ssl_body_t * body)
{
    LOG_INFO("SSL Header: Length=%u, Version=%u, Type=%u, Time=%ld, ", HEADER_LENGTH_BYTE, header->version, header->type, (long)header->time);
    LOG_INFO("SSL Body: Length=%u, Data=%s", body->size, body->data);
}
void cleanup(void *arg)
{
    socket_fd_t client_socket = (socket_fd_t)(intptr_t)arg;
    close_socket(client_socket);
}
void my_ssl_communicate(void *arg)
{
    // =============自建ssl数据================
    socket_fd_t client_socket = (socket_fd_t)(intptr_t)arg;

    uint8_t *buffer = (uint8_t *) malloc(MAX_BUFFER_SIZE);
    memset(buffer, 0, MAX_BUFFER_SIZE);
    if (buffer == NULL) {
        LOG_ERROR("alloc failed.");
        exit(EXIT_FAILURE);
    }
    ssl_body_t *body_ptr ;
    ssl_header_t *header_ptr = (ssl_header_t *) malloc(sizeof(ssl_header_t));
    if (header_ptr == NULL) {
        LOG_ERROR("alloc failed.");
        exit(EXIT_FAILURE);
    }
    int ret = 0;
    while (1) {
        /* code */
        size_t size = receive_data(client_socket, buffer, MAX_BUFFER_SIZE);
        if (size == 0) {
            LOG_INFO("Receive no data.");
            break;
        }
        ret = fill_ssl_header(buffer, size, header_ptr);
        if (ret != SUCCESS) {
            LOG_ERROR("fill ssl header failed.");
            continue;
        }
        body_ptr = (ssl_body_t * ) malloc(sizeof(ssl_body_t) + header_ptr->len);
        fill_ssl_body(&buffer[HEADER_LENGTH_BYTE], header_ptr, body_ptr);

        if (ret != SUCCESS) {
            LOG_ERROR("fill ssl body failed.");
            continue;
        }
        print_ssl(header_ptr, body_ptr);
        free(body_ptr);
    }
    // 与当前client 断链
    close_socket(client_socket);
    free(header_ptr);
}
int handle_data(gnutls_session_t session)
{
    int ret;
    char buffer[MAX_BUF + 1];
    for (;;) {
        LOOP_CHECK(ret, gnutls_record_recv(session, buffer,
                                           MAX_BUF));
        printf("recv : %s.", buffer);
        if (ret == 0) {
            printf("\n- Peer has closed the GnuTLS connection\n");
            break;
        } else if (ret < 0 && gnutls_error_is_fatal(ret) == 0) {
            fprintf(stderr, "*** Warning: %s\n",
                    gnutls_strerror(ret));
        } else if (ret < 0) {
            fprintf(stderr,
                    "\n*** Received corrupted "
                    "data(%d). Closing the connection.\n\n",
                    ret);
            break;
        } else if (ret > 0) {
            /* echo data back to the client
             */
            CHECK(gnutls_record_send(session, buffer, ret));
        }
    }
    return ret;
}
void close_and_free(gnutls_session_t session, socket_fd_t  sock_fd)
{
    // 关闭连接
    gnutls_bye(session, GNUTLS_SHUT_RDWR);
    close_socket(sock_fd);
    gnutls_deinit(session);
    gnutls_global_deinit();
}
void handle_ssl(void *arg)
{
    socket_fd_t client_socket = (socket_fd_t)(intptr_t)arg;

    gnutls_session_t session;

    // 设置密码套件

    int ret;
    gnutls_init(&session, GNUTLS_SERVER);

    const char *suites = "NONE:+VERS-TLS1.0:+RSA:+AES-128-CBC:+SHA1:+SHA256:+MD5";
    gnutls_priority_set_direct(session, suites, NULL);

    gnutls_protocol_t protocol = gnutls_protocol_get_version(session);
    LOG_INFO("SERVER ssl version : %s", gnutls_protocol_get_name(protocol));
    gnutls_credentials_set(session, GNUTLS_CRD_CERTIFICATE, x509_cred);

    gnutls_transport_set_int(session, client_socket);

    ret = gnutls_handshake(session);
    if (ret < 0) {
        fprintf(stderr, "SERVER Handshake failed : %s \n", gnutls_strerror(ret));
        close_and_free(session, client_socket);
        return;
    }
    ret = handle_data(session);
    if (ret < 0) {
        close_and_free(session, client_socket);
    }

    // 关闭连接
    close_and_free(session, client_socket);
}

void handle_request(void *arg)
{
    pthread_cleanup_push(cleanup, arg);
    handle_ssl(arg);
    pthread_cleanup_pop(1);
}

int main(void )
{
    socket_fd_t listen_fd;
    int client_socket;
    listen_fd = create_server_socket(SSL_PORT);
    if (listen_fd < 0) {
        LOG_ERROR("create bind listen fd failed");
        return 0;
    }
    LOG_INFO("server listening on port (%d)", SSL_PORT);

    pthread_t client_thread;

    gnutls_global_init();
    // 设置证书

    gnutls_certificate_allocate_credentials(&x509_cred);
    gnutls_certificate_set_x509_system_trust (x509_cred);
    gnutls_certificate_set_x509_key_file(x509_cred, CERT_FILE, KEY_FILE,
                                         GNUTLS_X509_FMT_PEM);


    while (1) {
        client_socket = accept_client(listen_fd);
        if (client_socket < 0) {
            LOG_ERROR("accept failed. %s", strerror(errno));
            break;
        }

        if (pthread_create(&client_thread, NULL, (void *)handle_request,(void *)(intptr_t)client_socket) != 0) {
            LOG_ERROR("pthread_create failed. %s", strerror(errno));
            close_socket(client_socket);
            continue;
        }
    }
    close_socket(client_socket);
    return 0;

}