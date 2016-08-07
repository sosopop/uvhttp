#include "uvhttp_server.h"
#include "uvhttp_base.h"
#include "uvhttp_server_internal.h"
#include <stdarg.h>

static void server_session_connected(
    uv_stream_t* stream,
    int status
    );

static void session_data_alloc( 
    uv_handle_t* handle,
    size_t suggested_size,
    uv_buf_t* buf
    );

static void session_close( 
    struct uvhttp_session_obj* session_obj
    );

static void session_data_read(
    uv_stream_t* stream,
    ssize_t nread,
    const uv_buf_t* buf
    );

static void session_delete( 
    struct uvhttp_session_obj* session_obj
    );

int uvhttp_server_set_option( 
    uvhttp_server server,
    uvhttp_server_option option,
    ...
    )
{
    int ret = 0;
    struct uvhttp_server_obj* server_obj = (struct uvhttp_server_obj*)server;
    va_list ap;
    va_start(ap, option);
    switch( option)
    {
    case UVHTTP_SRV_OPT_USER_DATA:
        server_obj->user_data = va_arg(ap, void*);
        break;
    case UVHTTP_SRV_OPT_SESSION_NEW_CB:
        server_obj->session_new_callback = va_arg(ap, uvhttp_server_session_new_callback);
        break;
    case UVHTTP_SRV_OPT_END_CB:
        server_obj->end_callback = va_arg(ap, uvhttp_server_end_callback);
        break;
    default:
        ret = UVHTTP_ERROR_NOOPTIONS;
        break;
    }
    va_end(ap);
    return ret;
}

int uvhttp_session_set_option( 
    uvhttp_session session,
    uvhttp_session_option option,
    ...
    )
{
    int ret = 0;
    struct uvhttp_session_obj* session_obj = (struct uvhttp_session_obj*)session;
    va_list ap;
    va_start(ap, option);
    switch( option)
    {
    case UVHTTP_SESSION_OPT_USER_DATA:
        session_obj->user_data = va_arg(ap, void*);
        break;
    case UVHTTP_SESSION_OPT_REQUEST_CB:
        session_obj->request_callback = va_arg(ap, uvhttp_session_request_callback);
        break;
    case UVHTTP_SESSION_OPT_REQUEST_BODY_CB:
        session_obj->body_read_callback = va_arg(ap, uvhttp_session_body_read_callback);
        break;
    case UVHTTP_SESSION_OPT_REQUEST_END_CB:
        session_obj->request_end_callback = va_arg(ap, uvhttp_session_request_end_callback);
        break;
    case UVHTTP_SESSION_OPT_WRITE_CB:
        session_obj->write_callback = va_arg(ap, uvhttp_session_write_callback);
        break;
    case UVHTTP_SESSION_OPT_END_CB:
        session_obj->end_callback = va_arg(ap, uvhttp_session_end_callback);
        break;
    default:
        ret = UVHTTP_ERROR_NOOPTIONS;
        break;
    }
    va_end(ap);
    return ret;
}

int uvhttp_server_get_info( 
    uvhttp_server server,
    uvhttp_server_info info,
    ...
    )
{
    int ret = 0;
    struct uvhttp_server_obj* server_obj = (struct uvhttp_server_obj*)server;
    va_list ap;
    va_start(ap, info);
    switch( info)
    {
    case UVHTTP_SRV_INFO_USER_DATA:
        {
            *va_arg(ap, void**) = server_obj->user_data;
        }
        break;
    case UVHTTP_SRV_INFO_UVTCP:
        {
            *va_arg(ap, uv_tcp_t**) = (uvhttp_loop)server_obj->tcp;
        }
        break;
    case UVHTTP_SRV_INFO_LOOP:
        {
            *va_arg(ap, uvhttp_loop*) = (uvhttp_loop)server_obj->loop;
        }
        break;
    default:
        ret = UVHTTP_ERROR_NOOPTIONS;
        break;
    }
    va_end(ap);
    return ret;
}

int uvhttp_session_get_info( 
    uvhttp_session session,
    uvhttp_session_info info,
    ...
    )
{
    int ret = 0;
    struct uvhttp_session_obj* session_obj = (struct uvhttp_session_obj*)session;
    va_list ap;
    va_start(ap, info);
    switch( info)
    {
    case UVHTTP_SESSION_INFO_USER_DATA:
        {
            *va_arg(ap, void**) = session_obj->user_data;
        }
        break;
    case UVHTTP_SESSION_INFO_UVTCP:
        {
            *va_arg(ap, uv_tcp_t**) = (uvhttp_loop)session_obj->tcp;
        }
        break;
    case UVHTTP_SESSION_INFO_LOOP:
        {
            *va_arg(ap, uvhttp_loop*) = (uvhttp_loop)session_obj->loop;
        }
        break;
    default:
        ret = UVHTTP_ERROR_NOOPTIONS;
        break;
    }
    va_end(ap);
    return ret;
}

uvhttp_server uvhttp_server_new(
    uvhttp_loop loop
    )
{
    struct uvhttp_server_obj* server_obj = (struct uvhttp_server_obj*)malloc( 
        sizeof(struct uvhttp_server_obj) );

    memset( server_obj, 0, sizeof(struct uvhttp_server_obj));
    server_obj->tcp = (uv_tcp_t*)malloc( sizeof(uv_tcp_t) );
    memset( server_obj->tcp, 0, sizeof(uv_tcp_t));
    server_obj->tcp->data = server_obj;
    server_obj->loop = (uv_loop_t*)loop;

    return server_obj;
}

int uvhttp_server_ip4_listen(
    uvhttp_server server,
    const char* ip,
    int port
    )
{
    struct sockaddr_in addr;
    struct uvhttp_server_obj* server_obj = (struct uvhttp_server_obj*)server;
    int ret = uv_tcp_init( server_obj->loop, server_obj->tcp);
    if ( ret != 0)
        goto cleanup;
    ret = uv_ip4_addr( ip, port, &addr);
    if ( ret != 0)
        goto cleanup;
    ret = uv_tcp_bind( server_obj->tcp, (const struct sockaddr*)&addr, 0);
    if ( ret != 0)
        goto cleanup;
    ret = uv_tcp_nodelay( server_obj->tcp, 1);
    if ( ret != 0)
        goto cleanup;
    ret = uv_listen((uv_stream_t*)server_obj->tcp, 1024, server_session_connected);
    if ( ret != 0)
        goto cleanup;
cleanup:
    return ret;
}

static void server_session_connected(
    uv_stream_t* stream,
    int status
    ) 
{
    int ret = 0;
    struct uvhttp_server_obj* server_obj = 0;
    struct uvhttp_session_obj* session_obj = 0;

    server_obj = (struct uvhttp_server_obj*)stream->data;
    session_obj = (struct uvhttp_session_obj*)malloc( sizeof(struct uvhttp_session_obj));
    memset( session_obj, 0, sizeof(struct uvhttp_session_obj));
    session_obj->tcp = (uv_tcp_t*)malloc( sizeof(uv_tcp_t) );
    memset( session_obj->tcp, 0, sizeof(uv_tcp_t)); 
    session_obj->tcp->data = session_obj;

    http_parser_init( &session_obj->parser, HTTP_REQUEST);
    session_obj->server_obj = server_obj;
    session_obj->loop = server_obj->loop;

    ret = uv_tcp_init( session_obj->loop, session_obj->tcp);
    if ( ret != 0) {
        session_delete( session_obj);
        return;
    }
    ret = uv_tcp_nodelay( session_obj->tcp, 1);
    if ( ret != 0)
        goto cleanup;
    ret = uv_accept( stream, (uv_stream_t*)session_obj->tcp);
    if ( ret != 0)
        goto cleanup;
    ret = uv_read_start((uv_stream_t*)session_obj->tcp, session_data_alloc, session_data_read);
    if ( ret != 0)
        goto cleanup;

    if ( server_obj->session_new_callback) {
        server_obj->session_new_callback( server_obj, session_obj);
    }
cleanup:
    if ( ret != 0)
    {
        if ( session_obj)
            session_close( session_obj);
    }
}

static void session_data_alloc( 
    uv_handle_t* handle,
    size_t suggested_size,
    uv_buf_t* buf
    )
{
}

static void session_data_read(
    uv_stream_t* stream,
    ssize_t nread,
    const uv_buf_t* buf
    )
{
}

static void session_close( 
    struct uvhttp_session_obj* session_obj
    )
{
}

static void session_delete( 
    struct uvhttp_session_obj* session_obj
    )
{
    UVHTTP_SAFE_FREE( session_obj->tcp);
    free( session_obj);
}
