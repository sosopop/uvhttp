#ifndef UVHTTP_INTERNAL_H__
#define UVHTTP_INTERNAL_H__

#include <uv.h>
#include "uvhttp_internal.h"
#include "uvhttp_util.h"
#include "uvhttp_base.h"
#include "uvhttp_server.h"
#include "http_parser.h"

#if defined(__cplusplus)
extern "C" {
#endif

struct uvhttp_server_obj {
    void* user_data;
    uv_loop_t* loop;
    uv_tcp_t* tcp;
    uvhttp_server_request_callback request_callback;
    uvhttp_server_end_callback end_callback;
    unsigned char deleted:1;
} ;

struct  uvhttp_session_obj {
    void* user_data;
    uv_loop_t* loop;
    uv_tcp_t* tcp;
    uvhttp_session_body_read_callback read_callback;
    uvhttp_session_end_callback end_callback;
    struct uvhttp_server_obj* server;

    struct uvhttp_message request;
    http_parser parser;
    unsigned char header_complete:1;
    char* net_buffer_in[UVHTTP_NET_BUFFER_SIZE];
    unsigned int net_buffer_in_len;
    char* net_buffer_out[UVHTTP_NET_BUFFER_SIZE];
    unsigned int net_buffer_out_len;
};

#if defined(__cplusplus)
}
#endif /* __cplusplus */
#endif // UVHTTP_INTERNAL_H__