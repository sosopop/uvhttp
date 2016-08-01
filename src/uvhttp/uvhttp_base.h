#ifndef uvhttp_base_h__
#define uvhttp_base_h__
#include "uvhttp_util.h"

#if defined(__cplusplus)
extern "C" {
#endif

#define UVHTTP_MAX_HEADERS_SIZE 40
typedef void* uvhttp_loop;

typedef enum {
    UVHTTP_ERROR_OK,
    UVHTTP_ERROR_FAILED
} uvhttp_error_code;

struct uvhttp_header {
    struct uvhttp_chunk key;
    struct uvhttp_chunk value;
};

struct uvhttp_message{
    int resp_code;
    long long content_length;
    struct uvhttp_chunk resp_status;
    struct uvhttp_chunk method;
    struct uvhttp_chunk uri;
    struct uvhttp_chunk proto;
    struct uvhttp_header headers[UVHTTP_MAX_HEADERS_SIZE];
    struct uvhttp_chunk body;
};

const char* uvhttp_error_description(
    int error_no
    );

uvhttp_loop uvhttp_loop_new( );

void uvhttp_loop_delete( 
    uvhttp_loop loop
    );

int uvhttp_run( 
    uvhttp_loop loop
    );

void uvhttp_stop(
    uvhttp_loop loop
    );

#if defined(__cplusplus)
}
#endif /* __cplusplus */
#endif // uvhttp_base_h__