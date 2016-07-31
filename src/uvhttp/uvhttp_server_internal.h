#ifndef UVHTTP_INTERNAL_H__
#define UVHTTP_INTERNAL_H__

#include "uvhttp.h"
#include "http_parser.h"
#include "uvhttp_util.h"

#if defined(__cplusplus)
extern "C" {
#endif
#define MAX_DATA_BUFFER 4*1024

struct uvhttp_srv_obj {
    void* user_data;
    uv_loop_t* loop;
    uv_tcp_t* tcp;
    uvhttp_request_cb request_cb;
    uvhttp_end_cb end_cb;
    unsigned char deleted:1;
} ;

struct  uvhttp_srv_session_obj {
    void* user_data;
    uv_loop_t* loop;
    uv_tcp_t* tcp;
    uvhttp_read_cb read_cb;
    uvhttp_end_cb end_cb;
    struct uvhttp_request request;
    http_parser parser;
    struct uvhttp_srv_obj* srv;
    unsigned char header_complete:1;
    struct uvhttp_buffer raw_buf;
    unsigned char temp_buf[MAX_DATA_BUFFER];
};

struct uvhttp_header_obj {
    uv_buf_t name;
    uv_buf_t value;
    struct uvhttp_header_obj* next;
};
#if defined(__cplusplus)
}
#endif /* __cplusplus */
#endif // UVHTTP_INTERNAL_H__