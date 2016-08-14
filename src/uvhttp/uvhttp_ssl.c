#include "uvhttp_ssl.h"
static int uvhttp_ssl_listen( 
    uv_stream_t* stream,
    int backlog, 
    uv_connection_cb cb
    )
{
    int ret = 0;

    return ret;
}

static int uvhttp_ssl_read_start(
    uv_stream_t* stream,
    uv_alloc_cb alloc_cb,
    uv_read_cb read_cb
    )
{
    int ret = 0;

    return ret;
}

static int uvhttp_ssl_write(uv_write_t* req,
    uv_stream_t* handle,
    const uv_buf_t bufs[],
    unsigned int nbufs,
    uv_write_cb cb)
{
    int ret = 0;

    return ret;
}