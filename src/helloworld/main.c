#include <stdio.h>
#include "uvhttp.h"

static void uvhttp_session_request(
    uvhttp_session session,
    struct uvhttp_message* request
    )
{
    struct uvhttp_header* list = request->headers;
    struct uvhttp_header* end = 0;
    struct uvhttp_header* iter = 0;
    printf( "session request url: %s\n", request->uri);
    printf( "session headers:\n");
    iter = uvhttp_headers_begin( list);
    end = uvhttp_headers_end( list);

    for ( ; iter !=end ; iter = iter->next ) {
        printf( "%s: %s\n", iter->field, iter->value);
    }
}

static void uvhttp_session_body_read(
    uvhttp_session session,
    struct uvhttp_chunk data
    )
{
    printf( "request body: %.*s\n", data.len, data.base);
}

static void uvhttp_session_request_end(
    int status,
    uvhttp_session session
    )
{
    printf( "session request end!\n");
}

static void uvhttp_session_end(
    uvhttp_session session
    )
{
    printf( "session end!\n");
}

static void uvhttp_server_session_new(
    uvhttp_server server,
    uvhttp_session session
    )
{
    uvhttp_session_set_option( session, UVHTTP_SESSION_OPT_REQUEST_CB, uvhttp_session_request);
    uvhttp_session_set_option( session, UVHTTP_SESSION_OPT_REQUEST_BODY_CB, uvhttp_session_body_read);
    uvhttp_session_set_option( session, UVHTTP_SESSION_OPT_REQUEST_END_CB, uvhttp_session_request_end);
    uvhttp_session_set_option( session, UVHTTP_SESSION_OPT_END_CB, uvhttp_session_end);
    printf( "new session created!\n");
}

int main(int argc, char* argv[])
{
    uvhttp_loop loop = uvhttp_loop_new();
    if ( loop) {
        uvhttp_server server = uvhttp_server_new( loop);
        if ( server) {
            uvhttp_server_set_option( server, UVHTTP_SRV_OPT_SESSION_NEW_CB, uvhttp_server_session_new);
            if ( uvhttp_server_ip4_listen( server, "0.0.0.0", 8011) == UVHTTP_OK) {
            }
        }
        uvhttp_run( loop);
        uvhttp_server_delete( server);
        uvhttp_loop_delete( loop);
    }
    return 0;
}
