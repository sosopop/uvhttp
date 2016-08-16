#include "uvhttp_ssl.h"
#include "mbedtls/certs.h"


static void uvhttp_ssl_read_cb(
    uv_stream_t* stream,
    ssize_t nread,
    const uv_buf_t* buf
    )
{
    struct uvhttp_ssl* ssl = (struct uvhttp_ssl*)stream;
    ssl->read_cb( stream, nread, buf);
}

static void uvhttp_ssl_alloc_cb(
    uv_handle_t* handle,
    size_t suggested_size,
    uv_buf_t* buf
    )
{
    struct uvhttp_ssl* ssl = (struct uvhttp_ssl*)handle;
    ssl->alloc_cb( handle, suggested_size, buf);
}

int uvhttp_ssl_listen( 
    uv_stream_t* stream,
    int backlog, 
    uv_connection_cb cb
    )
{
    int ret = 0;

    return ret;
}

static int ssl_recv( 
    void *ctx, 
    unsigned char *buf,
    size_t len
    )
{
    int ret = 0;
    return ret ;
}

static int ssl_send(
    void *ctx, 
    const unsigned char *buf, 
    size_t len
    )
{
    return len;
}

int uvhttp_ssl_init(
    uv_loop_t* loop,
    uv_tcp_t* handle
    )
{
    int ret = 0;
    struct uvhttp_ssl* ssl = (struct uvhttp_ssl*)handle;
    
    mbedtls_ssl_init( &ssl->ssl );
    mbedtls_ssl_config_init( &ssl->conf );
    mbedtls_x509_crt_init( &ssl->cacert );
    mbedtls_ctr_drbg_init( &ssl->ctr_drbg );
    mbedtls_entropy_init( &ssl->entropy );

    if( ( ret = mbedtls_ctr_drbg_seed( &ssl->ctr_drbg, mbedtls_entropy_func, &ssl->entropy,
                               (const unsigned char *) "UVHTTP",
                               sizeof( "UVHTTP" ) -1) ) != 0 ) {
        goto cleanup;
    }

    ret = mbedtls_x509_crt_parse( &ssl->cacert, (const unsigned char *) mbedtls_test_cas_pem,
                          mbedtls_test_cas_pem_len );
    if( ret < 0 ) {
        goto cleanup;
    }

    if( ( ret = mbedtls_ssl_config_defaults( &ssl->conf,
                    MBEDTLS_SSL_IS_CLIENT,
                    MBEDTLS_SSL_TRANSPORT_STREAM,
                    MBEDTLS_SSL_PRESET_DEFAULT ) ) != 0 ) {
        goto cleanup;
    }

    mbedtls_ssl_conf_authmode( &ssl->conf, MBEDTLS_SSL_VERIFY_OPTIONAL );
    mbedtls_ssl_conf_ca_chain( &ssl->conf, &ssl->cacert, NULL );
    mbedtls_ssl_conf_rng( &ssl->conf, mbedtls_ctr_drbg_random, &ssl->ctr_drbg );
    //mbedtls_ssl_conf_dbg( &clt->ssl_ctx->conf, my_debug, stdout );
    //mbedtls_debug_set_threshold( 1 );

    if( ( ret = mbedtls_ssl_setup( &ssl->ssl, &ssl->conf ) ) != 0 ) {
        goto cleanup;
    }

    if( ( ret = mbedtls_ssl_set_hostname( &ssl->ssl, "UVHTTP" ) ) != 0 ) {
        goto cleanup;
    }

    mbedtls_ssl_set_bio( &ssl->ssl, ssl, ssl_send, ssl_recv, NULL );

    ret = uv_tcp_init( loop, (uv_tcp_t*)&ssl->tcp);
    if ( ret != 0) {
        goto cleanup;
    }
cleanup:

    return ret;
}

int uvhttp_ssl_read_start(
    uv_stream_t* stream,
    uv_alloc_cb alloc_cb,
    uv_read_cb read_cb
    )
{
    int ret = 0;
    struct uvhttp_ssl* ssl = (struct uvhttp_ssl*)stream;
    ssl->read_cb = read_cb;
    ssl->alloc_cb = alloc_cb;
    ret = uv_read_start( stream, uvhttp_ssl_alloc_cb, uvhttp_ssl_read_cb);
    return ret;
}

int uvhttp_ssl_write(uv_write_t* req,
    uv_stream_t* handle,
    const uv_buf_t bufs[],
    unsigned int nbufs,
    uv_write_cb cb)
{
    int ret = 0;

    return ret;
}

static void uvhttp_ssl_close_cb(
    uv_handle_t* handle
    )
{
    struct uvhttp_ssl* ssl = (struct uvhttp_ssl*)handle;
    mbedtls_x509_crt_free( &ssl->cacert );
    mbedtls_ssl_free( &ssl->ssl );
    mbedtls_ssl_config_free( &ssl->conf );
    mbedtls_ctr_drbg_free( &ssl->ctr_drbg );
    mbedtls_entropy_free( &ssl->entropy );
    ssl->close_cb( handle);
}

void uvhttp_ssl_close(
    uv_handle_t* handle, 
    uv_close_cb close_cb
    )
{
    ((struct uvhttp_ssl*)handle)->close_cb = close_cb;
    uv_close( handle, close_cb);
}