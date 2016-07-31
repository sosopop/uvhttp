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

struct  uvhttp_client_obj {
    void* user_data;
    uv_loop_t* loop;
    uv_tcp_t* tcp;

    struct uvhttp_message response;
    http_parser parser;
    unsigned char header_complete:1;
    struct uvhttp_buffer result_buffer;
    unsigned char net_buffer[UVHTTP_MAX_DATA_BUFFER_SIZE];
};

#if defined(__cplusplus)
}
#endif /* __cplusplus */
#endif // UVHTTP_INTERNAL_H__