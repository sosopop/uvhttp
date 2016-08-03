#ifndef UVHTTP_INTERNAL_H__
#define UVHTTP_INTERNAL_H__

#include <uv.h>
#include "uvhttp_internal.h"
#include "uvhttp_util.h"
#include "uvhttp_base.h"
#include "uvhttp_client.h"
#include "http_parser.h"

#if defined(__cplusplus)
extern "C" {
#endif

struct  uvhttp_client_obj {
    void* user_data;
    uv_loop_t* loop;
    uv_tcp_t* tcp;
    uvhttp_client_body_write_callback write_callback;
    uvhttp_client_body_read_callback read_callback;
    uvhttp_client_end_callback end_callback;
    uvhttp_client_response_callback end_callback;

    struct uvhttp_buffer request_buffer;
    struct uvhttp_message response;
    http_parser parser;
    struct uvhttp_buffer net_buffer_in;
    struct uvhttp_buffer net_buffer_out;

    unsigned char deleted:1;
};

#if defined(__cplusplus)
}
#endif /* __cplusplus */
#endif // UVHTTP_INTERNAL_H__