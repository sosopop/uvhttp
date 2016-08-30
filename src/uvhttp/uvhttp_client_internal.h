#ifndef UVHTTP_CLIENT_INTERNAL_H__
#define UVHTTP_CLIENT_INTERNAL_H__

#include <uv.h>
#include "uvhttp_internal.h"
#include "uvhttp_util.h"
#include "uvhttp_base.h"
#include "uvhttp_client.h"
#include "http_parser.h"

#if defined(__cplusplus)
extern "C" {
#endif

typedef enum
{
    UVHTTP_CLIENT_STATUS_INITED,
    UVHTTP_CLIENT_STATUS_RUNNING,
    UVHTTP_CLIENT_STATUS_CLOSED
} uvhttp_client_status;

struct  uvhttp_client_obj {
    void* user_data;
    uv_loop_t* loop;
    uv_tcp_t* tcp;
    uvhttp_client_body_write_callback write_callback;
    uvhttp_client_body_read_callback read_callback;
    uvhttp_client_end_callback end_callback;
    uvhttp_client_response_callback response_callback;

    struct uvhttp_buffer request_buffer;
    struct uvhttp_message response;
    http_parser parser;
    struct uvhttp_buffer net_buffer_in;
    struct uvhttp_buffer net_buffer_out;
    char* temp_header_field;
    char* temp_header_value;

    int last_error;
    unsigned char status:5;
    unsigned char response_finished:1;
    unsigned char deleted:1;
    unsigned char ssl:1;
};

#if defined(__cplusplus)
}
#endif /* __cplusplus */
#endif // UVHTTP_CLIENT_INTERNAL_H__