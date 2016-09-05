#include <stdio.h>
#include <string.h>
#include "unittest.h"
#include "uvhttp.h"
#include "http_parser.h"
#include "uvhttp_util.h"
#include "uvhttp_base.h"
#include "uvhttp_internal.h"

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

}

void client_response_end_callback(
    int status,
    uvhttp_client client
    )
{

}

void client_end_callback(
    uvhttp_client client
    )
{

}

void client_body_read_callback(
    uvhttp_client client,
    struct uvhttp_chunk data
    )
{

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
        char result[20] = {0};
        void* runptr = 0;
        TEST_EQ( loop);
        TEST_EQ( client);
        uvhttp_client_set_option( client, UVHTTP_CLT_OPT_RESPONSE_CB, client_response_end_callback);
        uvhttp_client_set_option( client, UVHTTP_CLT_OPT_RESPONSE_BODY_READ_CB, client_body_read_callback);
        uvhttp_client_set_option( client, UVHTTP_CLT_OPT_REQUEST_BODY_WRITE_CB, client_write_callback);
        uvhttp_client_set_option( client, UVHTTP_CLT_OPT_RESPONSE_END_CB, client_body_write_callback);
        uvhttp_client_set_option( client, UVHTTP_CLT_OPT_END_CB, client_end_callback);
        TEST_EQ( uvhttp_client_request( client, "http://www.youku.com", "GET", "User-Agent: UVHttp\r\n", 0) == UVHTTP_OK);
        uvhttp_run( loop);
        uvhttp_client_delete( client);
        uvhttp_loop_delete( loop);
        app_path( bat_file, UVHTTP_ARRAY_SIZE(bat_file), "..\\..\\..\\tools\\test06.bat");
        runptr = run_shell( bat_file);
        wait_run( runptr);

        //检测服务器返回结果是否正确
        app_path( result_file, UVHTTP_ARRAY_SIZE(result_file), "..\\..\\..\\tools\\test06.txt");
        test_file = fopen( result_file, "r");
        fseek( test_file, 0,  SEEK_END);
        test_file_size = ftell( test_file);
        TEST_EQ( test_file_size == 12);
        fseek( test_file, 0,  SEEK_SET);
        fread( result, 1, test_file_size, test_file);
        fclose( test_file);
        del_file( result_file);
        TEST_EQ( strcmp( result, "Hello World\n") == 0);
    }

    printf( "TEST: end\n");
}