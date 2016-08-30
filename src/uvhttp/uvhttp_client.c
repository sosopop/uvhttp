#include "uvhttp_client.h"
#include "uvhttp_base.h"
#include "uvhttp_client_internal.h"
#include "uvhttp_ssl_client.h"
#include "uvhttp_internal.h"
#include <stdarg.h>

uvhttp_client uvhttp_client_new(
    uvhttp_loop loop
    )
{
    struct uvhttp_client_obj* client_obj = (struct uvhttp_client_obj*)malloc( 
        sizeof(struct uvhttp_client_obj) );

    memset( client_obj, 0, sizeof(struct uvhttp_client_obj));
    client_obj->loop = (uv_loop_t*)loop;

    return client_obj;
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

    //是否已经接收完成request请求
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

static void session_error(
    struct uvhttp_client_obj* client_obj
    )
{
    if ( client_obj->response_finished == 0 && client_obj->end_callback) {
        if ( client_obj->last_error != UVHTTP_OK) {
            client_obj->end_callback(  client_obj->last_error, client_obj);
        }
        else {
            client_obj->end_callback(  UVHTTP_ERROR_FAILED, client_obj);
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
    session_error( client_obj);
    //关闭连接回调
    if ( client_obj->end_callback) {
        client_obj->end_callback( 0, client_obj);
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
    if ( client_obj->status == UVHTTP_CLIENT_STATUS_RUNNING ) {
        client_obj->deleted = 1;
        uvhttp_client_abort( client_obj);
    }
    else {
        client_delete( client_obj);
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
