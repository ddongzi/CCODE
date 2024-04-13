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
void close_and_free(gnutls_session_t session, socket_fd_t  sock_fd)
{
    // 关闭连接
    gnutls_bye(session, GNUTLS_SHUT_RDWR);
    close_socket(sock_fd);
    gnutls_deinit(session);
    gnutls_global_deinit();
}
void handle_ssl(socket_fd_t sock_fd)
{
    char buffer[MAX_BUF + 1], *desc;
    gnutls_datum_t out;
    int type;
    unsigned status;
    gnutls_certificate_credentials_t xcred;

    int ret;
    const char *err;
    const char *suites = "NONE:+VERS-TLS1.0:+RSA:+AES-128-CBC:+SHA1:+SHA256:+MD5";

    gnutls_global_init();
    gnutls_session_t session;

    /* X509 stuff */
    CHECK(gnutls_certificate_allocate_credentials(&xcred));

    /* sets the system trusted CAs for Internet PKI */
    CHECK(gnutls_certificate_set_x509_system_trust(xcred));

    gnutls_init(&session, GNUTLS_CLIENT);
    // GNUTLS_CRD_CERTIFICATE 表示只对服务端证书验证
    gnutls_credentials_set (session, GNUTLS_CRD_CERTIFICATE, xcred);

    gnutls_priority_set_direct(session, suites, &err);
    if (ret < 0) {
        fprintf(stderr, "Syntax error at: %s\n", err);
    } else {
        LOG_INFO("SET suites success.");
    }

    gnutls_server_name_set(session, GNUTLS_NAME_DNS,
                           HOST,
                           strlen(HOST));

    gnutls_protocol_t protocol = gnutls_protocol_get_version(session);
    LOG_INFO("CLIENT ssl version : %s", gnutls_protocol_get_name(protocol));

    gnutls_transport_set_int(session, sock_fd);
    gnutls_handshake_set_timeout(session, GNUTLS_DEFAULT_HANDSHAKE_TIMEOUT);
    /* Perform the TLS handshake
 */
    do {
        ret = gnutls_handshake(session);
    } while (ret < 0 && gnutls_error_is_fatal(ret) == 0);
    if (ret < 0) {
        if (ret == GNUTLS_E_CERTIFICATE_VERIFICATION_ERROR) {
            /* check certificate verification status */
            type = gnutls_certificate_type_get(session);
            status = gnutls_session_get_verify_cert_status(session);
            CHECK(gnutls_certificate_verification_status_print(
                    status, type, &out, 0));
            printf("cert verify output: %s\n", out.data);
            gnutls_free(out.data);
        }
        fprintf(stderr, "*** Handshake failed: %s\n",
                gnutls_strerror(ret));
        close_and_free(session, sock_fd);
        return;
    } else {
        desc = gnutls_session_get_desc(session);
        printf("- Session info: %s\n", desc);
        gnutls_free(desc);
    }

    close_and_free(session, sock_fd);

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