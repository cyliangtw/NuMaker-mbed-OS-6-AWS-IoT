#ifndef _MY_TLS_SOCKET_H_
#define _MY_TLS_SOCKET_H_

#include "mbed.h"
#include "TLSSocket.h"
#include "mbedtls_utils.h"

#if MBED_CONF_MY_TLSSOCKET_TLS_DEBUG_LEVEL > 0
#include "mbedtls/debug.h"
#endif

/* MyTLSSocket = TLSSocket + MQTT lib required timed read/write + debug thru console */
class MyTLSSocket : public TLSSocket
{
public:
    MyTLSSocket();
    ~MyTLSSocket();

    /**
     * Timed recv for MQTT lib
     */
    int read(unsigned char* buffer, int len, int timeout);

    /**
     * Timed send for MQTT lib
     */
    int write(unsigned char* buffer, int len, int timeout);
    
protected:
#if MBED_CONF_MY_TLSSOCKET_TLS_DEBUG_LEVEL > 0
    /**
     * Debug callback for Mbed TLS
     * Just prints on the USB serial port
     */
    static void my_debug(void *ctx, int level, const char *file, int line,
                         const char *str);

    /**
     * Certificate verification callback for Mbed TLS
     * Here we only use it to display information on each cert in the chain
     */
    static int my_verify(void *data, mbedtls_x509_crt *crt, int depth, uint32_t *flags);
#endif
};

#endif // _MY_TLS_SOCKET_H_
