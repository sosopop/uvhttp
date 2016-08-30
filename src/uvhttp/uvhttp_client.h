#ifndef UVHTTP_CLIENT_H__
#define UVHTTP_CLIENT_H__
#include "uvhttp_util.h"
#include "uvhttp_base.h"

#if defined(__cplusplus)
extern "C" {
#endif
    
typedef void* uvhttp_client;

typedef enum {
    UVHTTP_CLT_OPT_USER_DATA,
    UVHTTP_CLT_OPT_RESPONSE_CB,
    UVHTTP_CLT_OPT_BODY_READ_CB,
    UVHTTP_CLT_OPT_BODY_WRITE_CB,
    UVHTTP_CLT_OPT_END_CB,
    UVHTTP_CLT_OPT_REQUEST_HEADERS,
    UVHTTP_CLT_OPT_REQUEST_URL,
    UVHTTP_CLT_OPT_REQUEST_METHOD,
    UVHTTP_CLT_OPT_REQUEST_BODY
} uvhttp_client_opt_type;

typedef enum {
    UVHTTP_CLT_INFO_USER_DATA,
    UVHTTP_CLT_INFO_UVTCP,
    UVHTTP_CLT_INFO_LOOP
} uvhttp_client_info_type;

typedef void (*uvhttp_client_body_write_callback)(
    uvhttp_client client
    );

typedef void (*uvhttp_client_write_callback)(
    int status,
    uvhttp_client client
    );

typedef void (*uvhttp_client_response_callback)(
    uvhttp_client client,
    struct uvhttp_message* resp
    );

typedef void (*uvhttp_client_end_callback)(
    int status,
    uvhttp_client client
    );

typedef void (*uvhttp_client_body_read_callback)(
    uvhttp_client client
    );

uvhttp_client uvhttp_client_new(
    uvhttp_loop loop
    );

int uvhttp_client_request(
    uvhttp_client client
    );

void uvhttp_client_delete(
    uvhttp_client client
    );

int uvhttp_client_abort(
    uvhttp_client client
    );

int uvhttp_client_write(
    uvhttp_client client,
    struct uvhttp_chunk* data,
    uvhttp_client_body_write_callback write_callback 
    );

#if defined(__cplusplus)
}
#endif /* __cplusplus */
#endif // UVHTTP_CLIENT_H__
