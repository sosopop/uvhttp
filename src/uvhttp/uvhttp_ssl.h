#ifndef UVHTTP_SSL_H__
#define UVHTTP_SSL_H__
#include <uv.h>
#include "mbedtls/config.h"
#include "mbedtls/platform.h"
#include "mbedtls/entropy.h"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/ssl.h"
#include "uvhttp_internal.h"
#include "uvhttp_util.h"

struct uvhttp_ssl {
    uv_tcp_t tcp;
    uv_buf_t user_read_buf;
    uv_close_cb user_close_cb;
    uv_read_cb user_read_cb;
    uv_write_cb user_write_cb;
    uv_alloc_cb user_alloc_cb;
    char ssl_read_buffer[UVHTTP_NET_BUFFER_SIZE];
    int ssl_read_buffer_len;
    int ssl_read_buffer_offset;
    int ssl_write_index;
    uv_buf_t* ssl_write_bufs;
    unsigned int ssl_write_nbufs;
    uv_write_cb ssl_write_cb;
    char is_writing;
    mbedtls_pk_context key;
    mbedtls_entropy_context entropy;
    mbedtls_ctr_drbg_context ctr_drbg;
    mbedtls_ssl_context ssl;
    mbedtls_ssl_config conf;
    mbedtls_x509_crt srvcert;
};

int uvhttp_ssl_init(
    uv_loop_t* loop,
    uv_tcp_t* handle
    );

int uvhttp_ssl_connect(
    uv_connect_t* req,
    uv_tcp_t* handle,
    const struct sockaddr* addr,
    uv_connect_cb cb
    );

int uvhttp_ssl_read_start(
    uv_stream_t* stream,
    uv_alloc_cb alloc_cb,
    uv_read_cb read_cb
    );

int uvhttp_ssl_write(
    uv_write_t* req,
    uv_stream_t* handle,
    const uv_buf_t bufs[],
    unsigned int nbufs,
    uv_write_cb cb
    );

void uvhttp_ssl_close(
    uv_handle_t* handle, 
    uv_close_cb close_cb
);

#endif // UVHTTP_SSL_H__
