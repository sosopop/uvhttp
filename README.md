# UVHttp
UVHttp is an asynchronous HTTP framework written in C. The goal of uvhttp is to extend the libuv, and to support a small number of HTTP features.

UVHttp will teaching you how to create a simple http server and http client, before this I was very difficult to find a support c language, HTTPS, asynchronous framework of the HTTP server and the client's open source code, so I try to learn and develop the UVHttp, I hope to help you.

##Features
Cross platform<br>
HTTP keep-alive<br>
Non-blocking I/O<br>
HTTPS<br>
Server and client<br>

##Contact
QQ:12178761<br>
Email:12178761@qq.com<br>

##Sample
```c
static void uvhttp_session_request(
    uvhttp_session session,
    struct uvhttp_message* request
    )
{
    struct uvhttp_header* list = request->headers;
    struct uvhttp_header* end = 0;
    struct uvhttp_header* iter = 0;
#ifdef OUTPUT_DEUBGINFO
    printf( "session request url: %s\n", request->uri);
    printf( "session headers:\n");
#endif
    iter = uvhttp_headers_begin( list);
    end = uvhttp_headers_end( list);

    for ( ; iter !=end ; iter = iter->next ) {
#ifdef OUTPUT_DEUBGINFO
        printf( "%s: %s\n", iter->field, iter->value);
#endif
    }
}

static void uvhttp_session_body_read(
    uvhttp_session session,
    struct uvhttp_chunk data
    )
{
#ifdef OUTPUT_DEUBGINFO
    printf( "request body: %.*s\n", data.len, data.base);
#endif
}

 #define RESPONSE \
     "HTTP/1.1 200 OK\r\n" \
     "Content-Type: text/plain\r\n" \
     "Content-Length: 11\r\n" \
     "Connection: keep-alive\r\n" \
     "\r\n" \
     "Hello World"


void session_writed(
    int status,
    uvhttp_session session,
    void* user_data
    )
{
    uvhttp_session_abort( session);
}

static void uvhttp_session_request_end(
    int status,
    uvhttp_session session
    )
{
    struct uvhttp_chunk response;
    response.base = RESPONSE;
    response.len = sizeof(RESPONSE) - 1;
    uvhttp_session_write( session, &response, 0, session_writed);
#ifdef OUTPUT_DEUBGINFO
    printf( "session request end!\n");
#endif
}

static void uvhttp_session_end(
    uvhttp_session session
    )
{
    uvhttp_server server = 0;
    uvhttp_session_get_info( session, UVHTTP_SESSION_INFO_USER_DATA, &server);
#ifdef OUTPUT_DEUBGINFO
    printf( "session end!\n");
#endif
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
    uvhttp_session_set_option( session, UVHTTP_SESSION_OPT_USER_DATA, server);
#ifdef OUTPUT_DEUBGINFO
    printf( "new session created!\n");
#endif
}

int main(int argc, char* argv[])
{
    uvhttp_loop loop = uvhttp_loop_new();
    if ( loop) {
        uvhttp_server server_ssl = uvhttp_server_new( loop);
        uvhttp_server server = uvhttp_server_new( loop);
        if ( server) {
            uvhttp_server_set_option( server, UVHTTP_SRV_OPT_SESSION_NEW_CB, uvhttp_server_session_new);
            if ( uvhttp_server_ip4_listen( server, "0.0.0.0", 8011) == UVHTTP_OK) {
                printf("http on 8011 success\n");
            }
        }
        if ( server_ssl) {
            uvhttp_server_set_option( server_ssl, UVHTTP_SRV_OPT_SSL, 1);
            uvhttp_server_set_option( server_ssl, UVHTTP_SRV_OPT_SESSION_NEW_CB, uvhttp_server_session_new);
            if ( uvhttp_server_ip4_listen( server_ssl, "0.0.0.0", 8012) == UVHTTP_OK) {
                printf("https on 8012 success\n");
            }
        }
        uvhttp_run( loop);
        uvhttp_server_delete( server);
        uvhttp_server_delete( server_ssl);
        uvhttp_loop_delete( loop);
    }
    return 0;
}
```
