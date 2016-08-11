#include <stdio.h>
#include <string.h>
#include "unittest.h"
#include "uvhttp.h"
#include "http_parser.h"
#include "uvhttp_util.h"
#include "uvhttp_base.h"
#include "uvhttp_internal.h"

//后面还需要增加keepalive的测试

static char uvhttp_server_session_new_called = 0;
static char uvhttp_session_request_called = 0;
static char uvhttp_session_body_read_called = 0;
static char uvhttp_session_request_end_called = 0;
static char uvhttp_session_end_called = 0;
static char uvhttp_session_write_called = 0;
static char uvhttp_server_end_called = 0;
static char* request_body = 0;
static int request_count = 0;

static void session_writed(
    int status,
    uvhttp_session session,
    void* user_data
    )
{
    TEST_EQ( status == 0);
    uvhttp_session_write_called = 1;
    request_count ++;
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

    TEST_EQ( strcmp( request->uri, "/test") == 0);
    TEST_EQ( find_host);
    uvhttp_session_request_called = 1;
}

static void uvhttp_session_body_read(
    uvhttp_session session,
    struct uvhttp_chunk data
    )
{
    uvhttp_session_body_read_called = 1;
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
}

static void uvhttp_session_end(
    uvhttp_session session
    )
{
    uvhttp_server server = 0;
    uvhttp_session_request_end_called = 1;
    uvhttp_session_get_info( session, UVHTTP_SESSION_INFO_USER_DATA, &server);
    TEST_EQ( server);
    TEST_EQ( uvhttp_server_abort( server) == UVHTTP_OK);

    uvhttp_session_end_called = 1;
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
    uvhttp_server_session_new_called = 1;
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
    uvhttp_server_end_called = 1;
}

void do_test03(){
    printf( "TEST: uvhttp_server keep_alive test begin\n");
    {
        FILE* test_file = 0;
        int test_file_size = 0;
        uvhttp_loop loop = uvhttp_loop_new();
        uvhttp_server server = uvhttp_server_new( loop);
        char bat_file[250] = {0};
        char result_file[250] = {0};
        char result[20] = {0};
        void* runptr = 0;
        TEST_EQ( loop);
        TEST_EQ( server);
        uvhttp_server_set_option( server, UVHTTP_SRV_OPT_END_CB, uvhttp_server_end);
        uvhttp_server_set_option( server, UVHTTP_SRV_OPT_SESSION_NEW_CB, uvhttp_server_session_new);
        TEST_EQ( uvhttp_server_ip4_listen( server, "0.0.0.0", 8011) == UVHTTP_OK);
        app_path( bat_file, UVHTTP_ARRAY_SIZE(bat_file), "..\\..\\..\\tools\\test03.bat");
        runptr = run_shell( bat_file);
        uvhttp_run( loop);
        uvhttp_server_delete( server);
        uvhttp_loop_delete( loop);
        wait_run( runptr);
        TEST_EQ( uvhttp_server_session_new_called);
        TEST_EQ( uvhttp_session_request_called);
        TEST_EQ( uvhttp_session_body_read_called == 0);
        TEST_EQ( uvhttp_session_request_end_called);
        TEST_EQ( uvhttp_session_end_called);
        TEST_EQ( uvhttp_server_end_called);
        TEST_EQ( uvhttp_session_write_called);
        TEST_EQ( request_count == 100);
    }

    printf( "TEST: end\n");
}