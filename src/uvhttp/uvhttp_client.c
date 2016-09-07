#include "uvhttp_client.h"
#include "uvhttp_base.h"
#include "uvhttp_client_internal.h"
#include "uvhttp_ssl_client.h"
#include "uvhttp_internal.h"
#include <stdarg.h>

static void client_reset( 
    struct uvhttp_client_obj* client_obj
    );

static void client_close( 
    struct uvhttp_client_obj* client_obj
    );
    
static void client_error(
    struct uvhttp_client_obj* client_obj
    );



static int http_parser_on_client_message_begin(
    http_parser* parser
    )
{
    struct uvhttp_client_obj* client_obj = UVHTTP_CONTAINER_PTR( struct uvhttp_client_obj, parser, parser );
    client_reset( client_obj);
    return 0;
}

static int http_parser_on_client_status(
    http_parser* parser,
    const char *at,
    size_t length
    )
{
    struct uvhttp_client_obj* client_obj = UVHTTP_CONTAINER_PTR( struct uvhttp_client_obj, parser, parser );
    client_obj->response.resp_status = new_string_buffer( client_obj->response.resp_status, at, length);
    return 0;
}

static int http_parser_on_client_header_field(
    http_parser* parser,
    const char *at,
    size_t length
    )
{
    struct uvhttp_client_obj* client_obj = UVHTTP_CONTAINER_PTR( struct uvhttp_client_obj, parser, parser );
    if ( client_obj->temp_header_value) {
        client_obj->response.headers = uvhttp_headers_append( client_obj->response.headers, 
            client_obj->temp_header_field, client_obj->temp_header_value);
        client_obj->temp_header_field = 0;
        client_obj->temp_header_value = 0;
    }
    client_obj->temp_header_field = new_string_buffer( client_obj->temp_header_field, at, length);
    return 0;
}

static int http_parser_on_client_header_value(
    http_parser* parser,
    const char *at,
    size_t length
    )
{
    struct uvhttp_client_obj* client_obj = UVHTTP_CONTAINER_PTR( struct uvhttp_client_obj, parser, parser );
    client_obj->temp_header_value = new_string_buffer( client_obj->temp_header_value, at, length);
    return 0;
}

static int http_parser_on_client_headers_complete(
    http_parser* parser
    )
{
    struct uvhttp_client_obj* client_obj = UVHTTP_CONTAINER_PTR( struct uvhttp_client_obj, parser, parser );
    if ( client_obj->temp_header_value) {
        client_obj->response.headers = uvhttp_headers_append( client_obj->response.headers, 
            client_obj->temp_header_field, client_obj->temp_header_value);
        client_obj->temp_header_field = 0;
        client_obj->temp_header_value = 0;
    }
    client_obj->response.status_code = parser->status_code;
    client_obj->response.http_major = parser->http_major;
    client_obj->response.http_minor = parser->http_minor;
    client_obj->response.content_length = parser->content_length;
    client_obj->response.keep_alive = http_should_keep_alive(parser);
    if ( client_obj->response_callback) {
        client_obj->response_callback(
            client_obj,
            &client_obj->response
            );
    }
    return 0;
}

static int http_parser_on_client_body(
    http_parser* parser,
    const char *at,
    size_t length
    )
{
    struct uvhttp_client_obj* client_obj = UVHTTP_CONTAINER_PTR( struct uvhttp_client_obj, parser, parser );
    struct uvhttp_chunk chunk;
    chunk.base = (char*)at;
    chunk.len = length;
    client_obj->body_read_callback(
        client_obj,
        chunk
        );
    return 0;
}

static int http_parser_on_client_message_complete(
    http_parser* parser
    )
{
    struct uvhttp_client_obj* client_obj = UVHTTP_CONTAINER_PTR( struct uvhttp_client_obj, parser, parser );
    if ( client_obj->response_end_callback) {
        client_obj->response_end_callback( UVHTTP_OK, client_obj);
    }
    client_obj->response_finished = 1;
    return 0;
}

static int http_parser_on_client_chunk_header(
    http_parser* parser
    )
{
    return 0;
}

static int http_parser_on_client_chunk_complete(
    http_parser* parser
    )
{
    return 0;
}

static http_parser_settings client_parser_settings = {
    http_parser_on_client_message_begin,
    0,
    http_parser_on_client_status,
    http_parser_on_client_header_field,
    http_parser_on_client_header_value,
    http_parser_on_client_headers_complete,
    http_parser_on_client_body,
    http_parser_on_client_message_complete,
    http_parser_on_client_chunk_header,
    http_parser_on_client_chunk_complete
};

uvhttp_client uvhttp_client_new(
    uvhttp_loop loop
    )
{
    struct uvhttp_client_obj* client_obj = (struct uvhttp_client_obj*)malloc( 
        sizeof(struct uvhttp_client_obj) );

    memset( client_obj, 0, sizeof(struct uvhttp_client_obj));
    client_obj->loop = (uv_loop_t*)loop;
    client_obj->status = UVHTTP_CLIENT_STATUS_INITED;

    return client_obj;
}

static int uvhttp_parse_url(
    struct uvhttp_client_obj* client_obj,
    const char* url,
    struct uvhttp_chunk* host,
    struct uvhttp_chunk* port,
    struct uvhttp_chunk* path,
    unsigned char* ssl
    )
{
    int ret = 0;
    struct http_parser_url parser_url;
    http_parser_url_init( &parser_url);
    ret = http_parser_parse_url( url, strlen(url), 0, &parser_url);
    if ( ret !=0 ) {
        ret = UVHTTP_ERROR_URL_PARSE;
        goto cleanup;
    }

    if ( parser_url.field_data[UF_HOST].len > MAX_DOMAIN_SIZE - 1) {
        ret = UVHTTP_ERROR_HOSTNAME_TOOLONG;
        goto cleanup;
    }

    host->base = (char*)url + parser_url.field_data[UF_HOST].off;
    host->len = parser_url.field_data[UF_HOST].len;

    if ( parser_url.field_data[UF_SCHEMA].len == 4 &&
        !memcmp( (char*)url + parser_url.field_data[UF_SCHEMA].off, "http", 4)) {
            *ssl = 0;
    }else if ( parser_url.field_data[UF_SCHEMA].len == 5 &&
        !memcmp( (char*)url + parser_url.field_data[UF_SCHEMA].off, "https", 5)) {
            *ssl = 1;
    }
    else {
        ret = UVHTTP_ERROR_FAILED;
        goto cleanup;
    }

    if ( parser_url.port == 0) {
        if ( *ssl ) {
            port->base = "443";
            port->len = 3;
        }
        else {
            port->base = "80";
            port->len = 2;
        }
    }
    else {
        port->base = (char*)url + parser_url.field_data[UF_PORT].off;
        port->len = parser_url.field_data[UF_PORT].len;
    }

    path->base = (char*)url + parser_url.field_data[UF_PATH].off;
    path->len = parser_url.field_data[UF_PATH].len;
cleanup:
    return ret;
}

static int uvhttp_client_make_request(
    struct uvhttp_client_obj* client_obj,
    const char* method,
    const char* headers,
    struct uvhttp_chunk* body,
    struct uvhttp_chunk* host,
    struct uvhttp_chunk* port,
    struct uvhttp_chunk* path
    )
{
    int ret = UVHTTP_ERROR_FAILED;
    char contentLength[32];
    uvhttp_buffer_init( &client_obj->request_buffer, 1024);
    if ( uvhttp_buf_append( &client_obj->request_buffer, method, strlen(method)) == 0) {
        goto cleanup;
    }
    if ( uvhttp_buf_append( &client_obj->request_buffer, " ", 1) == 0) {
        goto cleanup;
    }
    if ( uvhttp_buf_append( &client_obj->request_buffer, path->base, path->len) == 0) {
        goto cleanup;
    }
    if ( uvhttp_buf_append( &client_obj->request_buffer, " ", 1) == 0) {
        goto cleanup;
    }
    if ( uvhttp_buf_append( &client_obj->request_buffer, "HTTP/1.1\r\nHost: ", sizeof("HTTP/1.1\r\nHost: ")-1) == 0) {
        goto cleanup;
    }
    if ( uvhttp_buf_append( &client_obj->request_buffer, " ", 1) == 0) {
        goto cleanup;
    }
    if ( uvhttp_buf_append( &client_obj->request_buffer, host->base, host->len) == 0) {
        goto cleanup;
    }
    if ( uvhttp_vcmp( port, "80") != 0) {
        if ( uvhttp_buf_append( &client_obj->request_buffer, ":", 1) == 0) {
            goto cleanup;
        }
        if ( uvhttp_buf_append( &client_obj->request_buffer, port->base, port->len) == 0) {
            goto cleanup;
        }
    }
    if ( uvhttp_buf_append( &client_obj->request_buffer, "\r\n", 2) == 0) {
        goto cleanup;
    }
    if ( body && body->len > 0) {
        sprintf( contentLength, "Content-Length: %u\r\n", body->len);
        if ( uvhttp_buf_append( &client_obj->request_buffer, contentLength, strlen(contentLength)) == 0) {
            goto cleanup;
        }
    }
    if ( headers) {
        if ( uvhttp_buf_append( &client_obj->request_buffer, headers, strlen(headers)) == 0) {
            goto cleanup;
        }
    }
    if ( uvhttp_buf_append( &client_obj->request_buffer, "\r\n", 2) == 0) {
        goto cleanup;
    }
    if ( body && body->len > 0) {
        if ( uvhttp_buf_append( &client_obj->request_buffer, body->base, body->len) == 0) {
            goto cleanup;
        }
    }
    ret = UVHTTP_OK;
cleanup:
    if ( ret != 0) {
        uvhttp_buffer_free( &client_obj->request_buffer);
    }
    return ret;
}

static void request_written(
    uv_write_t* req,
    int status
    )
{
    int ret = 0;
    struct uvhttp_client_obj* client_obj = (struct uvhttp_client_obj*)req->data;
    if ( status < 0 ) {
        ret = -1;
        goto cleanup;
    }
//     //如果有body则直接发送body中的数据
//     if( client_obj->body.len > 0) {
//         ret = _write_clt_body( clt);
//         if ( ret != 0) {
//             goto cleanup;
//         }
//     }
//     //如果有写入回掉，则调用写入回调@@@
//     else if( clt->write_cb) {
//         clt->write_cb( clt);
//     }
cleanup:
    if ( ret != 0) {
        client_obj->last_error = ret;
        client_error( client_obj);
    }
    if ( req != 0 )
        free( req);
}

static int write_client_request( 
    struct uvhttp_client_obj* client_obj
    )
{
    int ret = 0;
    uv_write_t* write_req = 0;
    uv_buf_t req_buf;

    if ( client_obj->ssl) {
    }
    else {
         write_req = (uv_write_t*)malloc( sizeof(uv_write_t));
         write_req->data = client_obj;
         req_buf = uv_buf_init( client_obj->request_buffer.base, client_obj->request_buffer.len);
         ret = uv_write( write_req, (uv_stream_t*)client_obj->tcp, &req_buf, 1, request_written);
         if ( ret != 0) {
             goto cleanup;
         }
    }
cleanup:
    if ( ret != 0) {
        if ( write_req)
            free( write_req);
    }
    return ret;
}
    
static void client_data_alloc( 
    uv_handle_t* handle,
    size_t suggested_size,
    uv_buf_t* buf
    )
{
    struct uvhttp_client_obj* client_obj = (struct uvhttp_client_obj*)handle->data;
    *buf = uv_buf_init(
        client_obj->net_buffer_in,
        UVHTTP_NET_BUFFER_SIZE
        );
}

static void client_shutdown(
    uv_shutdown_t* req, 
    int status
    )
{
    struct uvhttp_client_obj* client_obj = (struct uvhttp_client_obj*)req->data;
    client_close( client_obj);
    free( req);
}

static void client_data_read(
    uv_stream_t* stream,
    ssize_t nread,
    const uv_buf_t* buf
    )
{
    struct uvhttp_client_obj* client_obj = (struct uvhttp_client_obj*)stream->data;
    size_t nparsed = 0;

    if ( nread > 0) {
        nparsed = http_parser_execute(&client_obj->parser, &client_parser_settings, buf->base, nread);
        if ( client_obj->parser.http_errno != 0) {
            client_obj->last_error = UVHTTP_ERROR_HTTP_PARSER;
            goto error;
        }
        if ( client_obj->parser.upgrade) {
            client_obj->last_error = UVHTTP_ERROR_HTTP_PARSER;
            goto error;
        } else if (nparsed != nread) {
            client_obj->last_error = UVHTTP_ERROR_HTTP_PARSER;
            goto error;
        }
    }
    else if ( nread == 0) {
        goto read_more;
    }
    else if( nread == UV_EOF) {
        uv_shutdown_t* req = (uv_shutdown_t*)malloc(sizeof(uv_shutdown_t));
        req->data = client_obj;
        client_obj->last_error = UVHTTP_ERROR_PEER_CLOSED;
        uv_shutdown(req, (uv_stream_t*)client_obj->tcp, client_shutdown);
        goto error;
    }
    else if (nread == UV_ECONNRESET || nread == UV_ECONNABORTED) {
        client_obj->last_error = UVHTTP_ERROR_PEER_CLOSED;
        client_close( client_obj);
        goto error;
    }
    else if (nread == UV_ENOBUFS) {
        client_obj->last_error = UVHTTP_ERROR_NO_BUFFERS;
        goto error;
    }
read_more:
    ;
    return;
error:
    client_error( client_obj);
}

static int client_start_read( struct uvhttp_client_obj* client_obj )
{
    int ret = 0;
    http_parser_init( &client_obj->parser, HTTP_RESPONSE);
    ret = uv_read_start((uv_stream_t*)client_obj->tcp, client_data_alloc, client_data_read);
    if ( ret == UV_EALREADY) {
        ret = 0;
    }
    if ( ret != 0 ) {
        goto cleanup;
    }
cleanup:
    return ret;
}

static void client_connected(
    uv_connect_t* req, 
    int status
    )
{
    int ret = 0;
    struct uvhttp_client_obj* client_obj = (struct uvhttp_client_obj*)req->data;
    if ( status < 0 ) {
        ret = -1;
        goto cleanup;
    }
    ret = client_start_read( client_obj);
    if ( ret != 0) {
        goto cleanup;
    }
    ret = write_client_request( client_obj );
    if ( ret != 0 ) {
        goto cleanup;
    }
cleanup:
    if ( ret != 0) {
        client_obj->last_error = ret;
        client_error( client_obj);
    }
    if ( req )
        free( req);
}

static void client_getaddrinfo(
    uv_getaddrinfo_t* req,
    int status,
    struct addrinfo* res
    )
{
    int ret = 0;
    uv_connect_t* connect_req = 0;
    struct uvhttp_client_obj* client_obj = (struct uvhttp_client_obj*)req->data;

    char addr[17] = {'\0'};
    if (status < 0 ) {
        ret = -1;
        goto cleanup;
    }
    ret = uv_ip4_name((struct sockaddr_in*) res->ai_addr, addr, 16);
    if ( ret != 0)
        goto cleanup;
    connect_req = (uv_connect_t*)malloc(sizeof(uv_connect_t));
    connect_req->data = client_obj;
    if ( client_obj->ssl) {
    }
    else {
        ret = uv_tcp_connect( connect_req, client_obj->tcp, res->ai_addr, client_connected);
        if ( ret != 0)
            goto cleanup;
    }
cleanup:
    if ( ret != 0) {
        if ( connect_req)
            free( connect_req);
        client_obj->last_error = ret;
        client_error( client_obj);
    }
    if ( res) 
        uv_freeaddrinfo(res);
    if ( req)
        free( req);
}

static int client_getaddr( struct uvhttp_client_obj* client_obj) {
    struct addrinfo hints;
    uv_getaddrinfo_t* addr_info = 0;
    int ret = 0;
    addr_info = (uv_getaddrinfo_t*)malloc(sizeof(uv_getaddrinfo_t));
    addr_info->data = client_obj;

    hints.ai_family = PF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = 0;

    ret = uv_getaddrinfo( client_obj->loop, addr_info, client_getaddrinfo, client_obj->host, client_obj->port, &hints);

    if ( ret != 0)
        free( addr_info);

    return ret;
}

static int client_new_conn_request( struct uvhttp_client_obj* client_obj) {
    int ret = 0;
    if ( client_obj->ssl) {
    }
    else{
        client_obj->tcp = (uv_tcp_t*)malloc( sizeof(uv_tcp_t) );
    }
    memset( client_obj->tcp, 0, sizeof(uv_tcp_t));
    client_obj->tcp->data = client_obj;
    if ( (ret = uv_tcp_init( client_obj->loop, client_obj->tcp)) != 0) {
        goto cleanup;
    }
    if ( (ret = client_getaddr( client_obj)) != 0) {
        goto cleanup;
    }
cleanup:
    return ret;
}

int uvhttp_client_request(
    uvhttp_client client, 
    const char* url,
    const char* method,
    const char* header, 
    struct uvhttp_chunk* body
    )
{
    int ret = 0;
    struct uvhttp_chunk host = {0,0};
    struct uvhttp_chunk port = {0,0};
    struct uvhttp_chunk path = {0,0};
    int need_new_conn = 0;
    unsigned char ssl = 0;
    struct uvhttp_client_obj* client_obj = (struct uvhttp_client_obj*)client;
    if ((client_obj->status != UVHTTP_CLIENT_STATUS_INITED &&
        client_obj->status != UVHTTP_CLIENT_STATUS_REQUEST_END &&
        client_obj->status != UVHTTP_CLIENT_STATUS_CLOSED)||
        client_obj->deleted) {
        return UVHTTP_ERROR_REQUEST_NOTEND;
    }

    //清空状态数据
    client_reset( client_obj);

    //解析url
    ret = uvhttp_parse_url( 
        client_obj, 
        url,
        &host,
        &port,
        &path,
        &ssl
        );
    if ( ret != 0) {
        goto cleanup;
    }

    if ( path.len == 0) {
        path.base = "/";
        path.len = 1;
    }

    client_obj->ssl = ssl;

    ret = uvhttp_client_make_request(
        client_obj,
        method,
        header,
        body,
        &host,
        &port,
        &path
        );
    if ( ret != 0) {
        goto cleanup;
    }

    if ( (client_obj->status == UVHTTP_CLIENT_STATUS_REQUEST_END &&
        (memcmp( client_obj->host, host.base, host.len ) != 0 || 
        memcmp( client_obj->port, port.base, port.len ) != 0)) ||
        client_obj->status == UVHTTP_CLIENT_STATUS_CLOSED) {
            need_new_conn = 1;
    }
    memset( client_obj->host, 0, sizeof(client_obj->host));
    memset( client_obj->port, 0, sizeof(client_obj->port));
    memcpy( client_obj->host, host.base, host.len);
    memcpy( client_obj->port, port.base, port.len);

    if ( client_obj->status == UVHTTP_CLIENT_STATUS_INITED) {
        client_new_conn_request( client_obj);
    }
    else {
        if ( need_new_conn ) {
            //建立新的连接
            if ( client_obj->status == UVHTTP_CLIENT_STATUS_CLOSED ) {
                client_new_conn_request( client_obj);
            }
            else {
                client_obj->new_conn = 1;
                client_close( client_obj);
            }
        }
        else {
            ret = write_client_request( client_obj );
            if ( ret !=0 ) {
                goto cleanup;
            }
        }
    }
cleanup:
    return ret;
}

static void client_reset( 
    struct uvhttp_client_obj* client_obj
    )
{
    struct uvhttp_header* list = client_obj->response.headers;
    struct uvhttp_header* end = 0;
    struct uvhttp_header* iter = 0;

    if ( list) {
        iter = uvhttp_headers_begin( list);
        end = uvhttp_headers_end( list);

        for ( ; iter !=end ; iter = iter->next ) {
            free_string_buffer( iter->field);
            free_string_buffer( iter->value);
        }

        if ( client_obj->response.headers) {
            uvhttp_headers_free( client_obj->response.headers);
            client_obj->response.headers = 0;
        }
    }

    if ( client_obj->temp_header_field) {
        free_string_buffer( client_obj->temp_header_field);
        client_obj->temp_header_field = 0;
    }
    if ( client_obj->temp_header_value) {
        free_string_buffer( client_obj->temp_header_value);
        client_obj->temp_header_value = 0;
    }
    if ( client_obj->response.resp_status) {
        free_string_buffer( (char*)client_obj->response.resp_status);
        client_obj->response.resp_status = 0;
    }

    uvhttp_buffer_free( &client_obj->request_buffer);
    client_obj->response_finished = 0;
    client_obj->last_error = UVHTTP_OK;
}

static void client_delete( 
    struct uvhttp_client_obj* client_obj
    )
{
    client_reset( client_obj);
    UVHTTP_SAFE_FREE( client_obj->tcp);
    free( client_obj);
}

static void client_error(
    struct uvhttp_client_obj* client_obj
    )
{
    if ( client_obj->response_finished == 0 && client_obj->end_callback) {
        if ( client_obj->last_error != UVHTTP_OK) {
            client_obj->response_end_callback(  client_obj->last_error, client_obj);
        }
        else {
            client_obj->response_end_callback(  UVHTTP_ERROR_FAILED, client_obj);
        }
        client_obj->response_finished = 1;
    }
}

static void client_closed( 
    uv_handle_t* handle
    )
{
    struct uvhttp_client_obj* client_obj = (struct uvhttp_client_obj*)handle->data;
    //如果关闭连接之前有错误导致request_end没有调用，则先通知request_end
    client_error( client_obj);
    //关闭连接回调
    if ( client_obj->end_callback) {
        client_obj->end_callback( client_obj);
    }
    client_delete( client_obj);
}

static void client_close( 
    struct uvhttp_client_obj* client_obj
    )
{
    if ( uv_is_closing( (uv_handle_t*)client_obj->tcp) == 0) {
        //uv_read_stop( (uv_stream_t*)session_obj->tcp);
        if ( client_obj->ssl) {
            uvhttp_ssl_client_close( (uv_handle_t*)client_obj->tcp, client_closed);
        }
        else {
            uv_close( (uv_handle_t*)client_obj->tcp, client_closed);
        }
    }
}

void uvhttp_client_delete(
    uvhttp_client client
    )
{
    struct uvhttp_client_obj* client_obj = (struct uvhttp_client_obj*)client;
    if ( client_obj->deleted ) {
        return;
    }
    else if ( client_obj->status == UVHTTP_CLIENT_STATUS_CLOSED ||
        client_obj->status == UVHTTP_CLIENT_STATUS_INITED) {
        client_delete( client_obj);
    }
    else {
        client_obj->deleted = 1;
        uvhttp_client_abort( client_obj);
    }
}

int uvhttp_client_abort(
    uvhttp_client client
    )
{
    struct uvhttp_client_obj* client_obj = (struct uvhttp_client_obj*)client;
    client_close( client_obj);
    return UVHTTP_OK;
}

int uvhttp_client_set_option(
    uvhttp_client client, 
    uvhttp_client_option option,
    ...
    )
{
    int ret = 0;
    struct uvhttp_client_obj* client_obj = (struct uvhttp_client_obj*)client;
    va_list ap;
    va_start(ap, option);
    switch( option)
    {
    case UVHTTP_CLT_OPT_USER_DATA:
        client_obj->user_data = va_arg(ap, void*);
        break;
    case UVHTTP_CLT_OPT_RESPONSE_CB:
        client_obj->response_callback = va_arg(ap, uvhttp_client_response_callback);
        break;
    case UVHTTP_CLT_OPT_RESPONSE_BODY_READ_CB:
        client_obj->body_read_callback = va_arg(ap, uvhttp_client_body_read_callback);
        break;
    case UVHTTP_CLT_OPT_REQUEST_BODY_WRITE_CB:
        client_obj->body_write_callback = va_arg(ap, uvhttp_client_body_write_callback);
        break;
    case UVHTTP_CLT_OPT_RESPONSE_END_CB:
        client_obj->response_end_callback = va_arg(ap, uvhttp_client_response_end_callback);
        break;
    case UVHTTP_CLT_OPT_END_CB:
        client_obj->end_callback = va_arg(ap, uvhttp_client_end_callback);
        break;
    default:
        ret = UVHTTP_ERROR_NOOPTIONS;
        break;
    }
    va_end(ap);
    return ret;
}

int uvhttp_client_get_info(
    uvhttp_client client, 
    uvhttp_client_info info, 
    ...
    )
{
    int ret = 0;
    struct uvhttp_client_obj* client_obj = (struct uvhttp_client_obj*)client;
    va_list ap;
    va_start(ap, info);
    switch( info)
    {
    case UVHTTP_CLT_INFO_USER_DATA:
        {
            *va_arg(ap, void**) = client_obj->user_data;
        }
        break;
    case UVHTTP_CLT_INFO_UVTCP:
        {
            *va_arg(ap, uv_tcp_t**) = (uvhttp_loop)client_obj->tcp;
        }
        break;
    case UVHTTP_CLT_INFO_LOOP:
        {
            *va_arg(ap, uvhttp_loop*) = (uvhttp_loop)client_obj->loop;
        }
        break;
    default:
        ret = UVHTTP_ERROR_NOOPTIONS;
        break;
    }
    va_end(ap);
    return ret;
}
