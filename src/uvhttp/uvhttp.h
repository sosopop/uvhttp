#ifndef UVHTTP_H__
#define UVHTTP_H__
#include <uv.h>

#if defined(__cplusplus)
extern "C" {
#endif

typedef void* uvhttp_obj;

typedef enum {
    UVHTTP_SRV_OPT_USER_DATA,
    UVHTTP_SRV_OPT_REQ_CB,
    UVHTTP_SRV_OPT_END_CB,
    UVHTTP_SESSION_OPT_USER_DATA,
    UVHTTP_SESSION_OPT_READ_CB,
    //每次请求必然有end，根据status判断是错误，还是成功，可以end的时候释放用户内存
    UVHTTP_SESSION_OPT_END_CB,
    UVHTTP_CLT_OPT_USER_DATA,
    UVHTTP_CLT_OPT_RESPONSE_CB,
    UVHTTP_CLT_OPT_READ_CB,
    UVHTTP_CLT_OPT_WRITE_CB,
    //每次请求必然有end，根据status判断是错误，还是成功，可以end的时候释放用户内存
    UVHTTP_CLT_OPT_END_CB
} uvhttp_opt_type;

typedef enum {
    UVHTTP_SRV_INFO_USER_DATA,
    UVHTTP_SESSION_INFO_USER_DATA,
    UVHTTP_SRV_INFO_TCP,
    UVHTTP_SESSION_INFO_TCP,
    UVHTTP_SRV_INFO_LOOP,
    UVHTTP_SESSION_INFO_LOOP,
    UVHTTP_SESSION_INFO_SRV
} uvhttp_info_type;

struct uvhttp_header{
    uv_buf_t name;
    uv_buf_t value;
    struct uvhttp_header* next;
};

struct uvhttp_request{
    uv_buf_t method;
    uv_buf_t uri;
    uv_buf_t proto;
    struct uvhttp_header* headers;
};

struct uvhttp_response {
    int resp_code;
    uv_buf_t resp_status;
    struct uvhttp_header* headers;
};


#if defined(__cplusplus)
}
#endif /* __cplusplus */
#endif // UVHTTP_H__