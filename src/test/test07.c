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
static int run_times = 0;

static void client_body_write_callback(
    uvhttp_client client
    )
{

}

void client_write_callback(
    int status,
    uvhttp_client client,
    void* user_data
    )
{

}

void client_response_callback(
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
    TEST_EQ( resp->content_length > 100);
    content_length = (int)resp->content_length;
    TEST_EQ( resp->keep_alive == 1);
    TEST_EQ( resp->http_major == 1);
    TEST_EQ( resp->http_minor == 1);
    TEST_EQ( resp->status_code == 200);
    client_response_called = 1;
}

void client_response_end_callback(
    int status,
    uvhttp_client client
    )
{
    client_response_end_called = 1;
    if ( run_times++ < 10) {
        TEST_EQ( uvhttp_client_request( client, "http://www.w3school.com.cn", "GET", "User-Agent: UVHttp\r\n", 0) == UVHTTP_OK);
    }
    else {
        uvhttp_client_abort( client);
    }
}

void client_end_callback(
    uvhttp_client client
    )
{
    client_end_called = 1;
}

void client_body_read_callback(
    uvhttp_client client,
    struct uvhttp_chunk data
    )
{
    response_body = new_string_buffer( response_body, data.base, data.len);
    client_body_read_called = 1;
}

void do_test06(){
    printf( "TEST: uvhttp_client test begin\n");
    {
        FILE* test_file = 0;
        int test_file_size = 0;
        uvhttp_loop loop = uvhttp_loop_new();
        uvhttp_client client = uvhttp_client_new( loop);
        char bat_file[250] = {0};
        char result_file[250] = {0};
        char result[1024] = {0};
        void* runptr = 0;
        TEST_EQ( loop);
        TEST_EQ( client);
        uvhttp_client_set_option( client, UVHTTP_CLT_OPT_RESPONSE_CB, client_response_callback);
        uvhttp_client_set_option( client, UVHTTP_CLT_OPT_RESPONSE_BODY_READ_CB, client_body_read_callback);
        uvhttp_client_set_option( client, UVHTTP_CLT_OPT_REQUEST_BODY_WRITE_CB, client_body_write_callback);
        uvhttp_client_set_option( client, UVHTTP_CLT_OPT_RESPONSE_END_CB, client_response_end_callback);
        uvhttp_client_set_option( client, UVHTTP_CLT_OPT_END_CB, client_end_callback);
        TEST_EQ( uvhttp_client_request( client, "http://www.w3school.com.cn", "GET", "User-Agent: UVHttp\r\n", 0) == UVHTTP_OK);
        uvhttp_run( loop);
        uvhttp_client_delete( client);
        uvhttp_loop_delete( loop);
        app_path( bat_file, UVHTTP_ARRAY_SIZE(bat_file), "..\\..\\..\\tools\\test06.bat");
        runptr = run_shell( bat_file);
        wait_run( runptr);

        TEST_EQ(client_response_called);
        TEST_EQ(client_response_end_called);
        TEST_EQ(client_body_read_called);
        TEST_EQ(client_end_called);

        //检测服务器返回结果是否正确
        app_path( result_file, UVHTTP_ARRAY_SIZE(result_file), "..\\..\\..\\tools\\test06.txt");
        test_file = fopen( result_file, "r");
        fseek( test_file, 0,  SEEK_END);
        test_file_size = ftell( test_file);
        TEST_EQ( test_file_size == content_length);
        fseek( test_file, 0,  SEEK_SET);
        fread( result, 1, min(test_file_size, 1023), test_file);
        fclose( test_file);
        del_file( result_file);
        TEST_EQ( strncmp( result, response_body, min(test_file_size, 1023)) == 0);

        if ( response_body ) {
            free_string_buffer( response_body);
            response_body = 0;
        }
    }

    printf( "TEST: end\n");
}