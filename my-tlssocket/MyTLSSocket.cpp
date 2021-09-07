#include "mbed.h"
#include "MyTLSSocket.h"

MyTLSSocket::MyTLSSocket()
{
    /* TLSSocket prints debug message thru mbed-trace. We override it and print thru STDIO. */
#if MBED_CONF_MY_TLSSOCKET_TLS_DEBUG_LEVEL > 0
    mbedtls_ssl_conf_verify(get_ssl_config(), my_verify, this);
    mbedtls_ssl_conf_dbg(get_ssl_config(), my_debug, this);
    mbedtls_debug_set_threshold(MBED_CONF_MY_TLSSOCKET_TLS_DEBUG_LEVEL);
#endif

    /* Enable RFC 6066 max_fragment_length extension in SSL */
#if defined(MBEDTLS_SSL_MAX_FRAGMENT_LENGTH) && (MBED_CONF_MY_TLSSOCKET_TLS_MAX_FRAG_LEN > 0)
    mbedtls_ssl_conf_max_frag_len(get_ssl_config(), MBED_CONF_MY_TLSSOCKET_TLS_MAX_FRAG_LEN);
#endif
}

MyTLSSocket::~MyTLSSocket()
{
}

int MyTLSSocket::read(unsigned char* buffer, int len, int timeout)
{
    set_timeout(timeout);

    int rc = recv(buffer, len);
    if (rc >= 0) {
        return rc;
    } else if (rc == NSAPI_ERROR_WOULD_BLOCK) {
        return 0;
    } else {
        printf("TLSSocket recv(%d) failed with %d\n", len, rc);
        return -1;
    }
}

int MyTLSSocket::write(unsigned char* buffer, int len, int timeout)
{
    set_timeout(timeout);

    int rc = send(buffer, len);
    if (rc >= 0) {
        return rc;
    } else if (rc == NSAPI_ERROR_WOULD_BLOCK) {
        return 0;
    } else {
        printf("TLSSocket send(%d) failed with %d\n", len, rc);
        return -1;
    }
}

#if MBED_CONF_MY_TLSSOCKET_TLS_DEBUG_LEVEL > 0
void MyTLSSocket::my_debug(void *ctx, int level, const char *file, int line,
                           const char *str)
{
    const char *p, *basename;
    MyTLSSocket *tlssocket = static_cast<MyTLSSocket *>(ctx);

    /* Extract basename from file */
    for (p = basename = file; *p != '\0'; p++) {
        if (*p == '/' || *p == '\\') {
            basename = p + 1;
        }
    }

    mbedtls_printf("%s:%04d: |%d| %s", basename, line, level, str);
}

int MyTLSSocket::my_verify(void *data, mbedtls_x509_crt *crt, int depth, uint32_t *flags)
{
    const uint32_t buf_size = 1024;
    char *buf = new char[buf_size];
    MyTLSSocket *tlssocket = static_cast<MyTLSSocket *>(data);

    printf("\nVerifying certificate at depth %d:\n", depth);
    mbedtls_x509_crt_info(buf, buf_size - 1, "  ", crt);
    printf("%s", buf);

    if (*flags == 0) {
        printf("No verification issue for this certificate\n");
    } else {
        mbedtls_x509_crt_verify_info(buf, buf_size, "  ! ", *flags);
        printf("%s\n", buf);
    }

    delete[] buf;

    return 0;
}
#endif
