#ifndef UVHTTP_SSL_H__
#define UVHTTP_SSL_H__
#include <uv.h>
#include "mbedtls/config.h"
#include "mbedtls/platform.h"
#include "mbedtls/entropy.h"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/ssl.h"

struct uvhttp_ssl_mbedtls {
    mbedtls_entropy_context entropy;
    mbedtls_ctr_drbg_context ctr_drbg;
    mbedtls_ssl_context ssl;
    mbedtls_ssl_config conf;
    mbedtls_x509_crt cacert;
};

struct uv_ssl {
    uv_tcp_t tcp;
    struct uvhttp_ssl_mbedtls* ssl;
};

static int uvhttp_ssl_connect(
    uv_connect_t* req,
    uv_tcp_t* handle,
    const struct sockaddr* addr,
    uv_connect_cb cb
    );

static int uvhttp_ssl_read_start(
    uv_stream_t* stream,
    uv_alloc_cb alloc_cb,
    uv_read_cb read_cb
    );

static int uvhttp_ssl_write(
    uv_write_t* req,
    uv_stream_t* handle,
    const uv_buf_t bufs[],
    unsigned int nbufs,
    uv_write_cb cb
    );

#endif // UVHTTP_SSL_H__
