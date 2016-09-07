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
    UVHTTP_CLIENT_STATUS_REQUESTING,
    UVHTTP_CLIENT_STATUS_REQUEST_END,
    UVHTTP_CLIENT_STATUS_CLOSING,
    UVHTTP_CLIENT_STATUS_CLOSED
} uvhttp_client_status;

struct  uvhttp_client_obj {
    void* user_data;
    uv_loop_t* loop;
    uv_tcp_t* tcp;
    uvhttp_client_body_write_callback body_write_callback;
    uvhttp_client_body_read_callback body_read_callback;
    uvhttp_client_end_callback end_callback;
    uvhttp_client_response_end_callback response_end_callback;
    uvhttp_client_response_callback response_callback;

    struct uvhttp_buffer request_buffer;
    struct uvhttp_message response;
    http_parser parser;
    char net_buffer_in[UVHTTP_NET_BUFFER_SIZE];
    char* temp_header_field;
    char* temp_header_value;

    char host[MAX_DOMAIN_SIZE];
    char port[PORT_SIZE];

    int last_error;
    unsigned char status:4;
    unsigned char response_finished:1;
    unsigned char deleted:1;
    unsigned char ssl:1;
    unsigned char new_conn:1;
};

#if defined(__cplusplus)
}
#endif /* __cplusplus */
#endif // UVHTTP_CLIENT_INTERNAL_H__