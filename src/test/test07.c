#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "unittest.h"
#include "uvhttp.h"
#include "http_parser.h"
#include "uvhttp_util.h"
#include "uvhttp_base.h"
#include "uvhttp_internal.h"

static char client_response_called = 0;
static char client_response_end_called = 0;
static char client_body_read_called = 0;
static char client_end_called = 0;
static char* response_body = 0;
static int content_length = 0;
static char* request_body = 0;

static void client_body_write_callback(
    uvhttp_client client
    )
{

}

static void client_write_callback(
    int status,
    uvhttp_client client,
    void* user_data
    )
{

}

static void client_response_callback(
    uvhttp_client client,
    struct uvhttp_message* resp
    )
{
    struct uvhttp_header* list = resp->headers;
    struct uvhttp_header* end = 0;
    struct uvhttp_header* iter = 0;
    iter = uvhttp_headers_begin( list);
    end = uvhttp_headers_end( list);
    for ( ; iter !=end ; iter = iter->next ) {
    }
    TEST_EQ( strcmp("OK", resp->resp_status) == 0);
    TEST_EQ( resp->content_length == 12);
    content_length = (int)resp->content_length;
    TEST_EQ( resp->keep_alive == 1);
    TEST_EQ( resp->http_major == 1);
    TEST_EQ( resp->http_minor == 1);
    TEST_EQ( resp->status_code == 200);
    client_response_called = 1;
}

static void client_response_end_callback(
    int status,
    uvhttp_client client
    )
{
    client_response_end_called = 1;
    uvhttp_client_abort( client);
}

static void client_end_callback(
    uvhttp_client client
    )
{
    client_end_called = 1;
}

static void client_body_read_callback(
    uvhttp_client client,
    struct uvhttp_chunk data
    )
{
    response_body = new_string_buffer( response_body, data.base, data.len);
    client_body_read_called = 1;
}
    
static void session_writed(
    int status,
    uvhttp_session session,
    void* user_data
    )
{
    TEST_EQ( status == 0);
    uvhttp_session_abort( session);
}

static void uvhttp_session_request(
    uvhttp_session session,
    struct uvhttp_message* request
    )
{
    struct uvhttp_header* list = request->headers;
    struct uvhttp_header* end = 0;
    struct uvhttp_header* iter = 0;
    char find_host = 0;
    iter = uvhttp_headers_begin( list);
    end = uvhttp_headers_end( list);

    for ( ; iter !=end ; iter = iter->next ) {
        if ( strcmp( iter->field, "Host") == 0) {
            if ( strcmp( iter->value, "127.0.0.1:8011") == 0) {
                find_host = 1;
            }
        }
    }

    TEST_EQ( strcmp( request->uri, "/test07") == 0);
    TEST_EQ( find_host);
}

static void uvhttp_session_body_read(
    uvhttp_session session,
    struct uvhttp_chunk data
    )
{
    request_body =  new_string_buffer( request_body, data.base, data.len);
}

#define RESPONSE \
    "HTTP/1.1 200 OK\r\n" \
    "Content-Type: text/plain\r\n" \
    "Content-Length: 12\r\n" \
    "Connection: keep-alive\r\n" \
    "\r\n" \
    "Hello World\n" 

static void uvhttp_session_request_end(
    int status,
    uvhttp_session session
    )
{
    struct uvhttp_chunk response;
    TEST_EQ( status == 0);
    response.base = RESPONSE;
    response.len = sizeof(RESPONSE) - 1;
    uvhttp_session_write( session, &response, 0, session_writed);
    TEST_EQ( strcmp( request_body, "name=test07") == 0);
}

static void uvhttp_session_end(
    uvhttp_session session
    )
{
    uvhttp_server server = 0;
    uvhttp_session_get_info( session, UVHTTP_SESSION_INFO_USER_DATA, &server);
    TEST_EQ( server);
    TEST_EQ( uvhttp_server_abort( server) == UVHTTP_OK);
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
}

static void uvhttp_server_end(
    int status,
    uvhttp_server server
    )
{
    uvhttp_loop loop = 0;
    TEST_EQ( status == 0);
    uvhttp_server_get_info( server, UVHTTP_SRV_INFO_LOOP, &loop);
    uvhttp_stop( loop);
}

void do_test07(){
    printf( "TEST: uvhttp_client test begin\n");
    {
        struct uvhttp_chunk http_request_body;
        uvhttp_loop loop = uvhttp_loop_new();
        uvhttp_client client = uvhttp_client_new( loop);
        uvhttp_server server = uvhttp_server_new( loop);
        TEST_EQ( loop);
        TEST_EQ( client);
        TEST_EQ( server);

        uvhttp_server_set_option( server, UVHTTP_SRV_OPT_END_CB, uvhttp_server_end);
        uvhttp_server_set_option( server, UVHTTP_SRV_OPT_SESSION_NEW_CB, uvhttp_server_session_new);
        TEST_EQ( uvhttp_server_ip4_listen( server, "0.0.0.0", 8011) == UVHTTP_OK);

        uvhttp_client_set_option( client, UVHTTP_CLT_OPT_RESPONSE_CB, client_response_callback);
        uvhttp_client_set_option( client, UVHTTP_CLT_OPT_RESPONSE_BODY_READ_CB, client_body_read_callback);
        uvhttp_client_set_option( client, UVHTTP_CLT_OPT_REQUEST_BODY_WRITE_CB, client_body_write_callback);
        uvhttp_client_set_option( client, UVHTTP_CLT_OPT_RESPONSE_END_CB, client_response_end_callback);
        uvhttp_client_set_option( client, UVHTTP_CLT_OPT_END_CB, client_end_callback);
        http_request_body.base = "name=test07";
        http_request_body.len = strlen( http_request_body.base);
        TEST_EQ( uvhttp_client_request( client, "http://127.0.0.1:8011/test07", "GET", "User-Agent: UVHttp\r\n", &http_request_body) == UVHTTP_OK);
        uvhttp_run( loop);
        uvhttp_client_delete( client);
        uvhttp_server_delete( server);
        uvhttp_loop_delete( loop);

        TEST_EQ(client_response_called);
        TEST_EQ(client_response_end_called);
        TEST_EQ(client_body_read_called);
        TEST_EQ(client_end_called);

        if ( response_body ) {
            free_string_buffer( response_body);
            response_body = 0;
        }
        if ( request_body) {
            free_string_buffer( request_body);
            request_body = 0;
        }
    }

    printf( "TEST: end\n");
}