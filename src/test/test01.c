#include <stdio.h>
#include <string.h>
#include "unittest.h"
#include "uvhttp.h"
#include "http_parser.h"
#include "uvhttp_util.h"
#include "uvhttp_base.h"
#include "uvhttp_internal.h"

void do_test01(){
    TEST_EQ(0==0);
    TEST_EQ(1==1);
    //≤‚ ‘string_buffer
    {
        char* str1 = new_cstring_buffer( "123", 0, 0);
        char* str2 = new_cstring_buffer( "!!!", 0, 0);
        char* str3 = 0;
        TEST_EQ( strcmp( str1, "123") == 0);
        free_string_buffer( str1);
        str1 = new_cstring_buffer( "123", "123456", 6 );
        TEST_EQ( strcmp( str1, "123123456") == 0);
        str3 = new_cstring_buffer( str2, str1, strlen(str1) );
        TEST_EQ( strcmp( str3, "!!!123123456") == 0)
        free_string_buffer( str1);
        free_string_buffer( str2);
        free_string_buffer( str3);
    }
    //≤‚ ‘uvhttp_list
    {
        struct uvhttp_header* list = 0;
        struct uvhttp_header* end = 0;
        struct uvhttp_header* iter = 0;
        int test_count = 0;
        int c = 0;

        list = uvhttp_headers_append( list , new_cstring_buffer( 0, "1", 1 ), new_cstring_buffer( 0, "2", 1 ) );
        TEST_EQ( uvhttp_headers_size(list) == 1);
        list = uvhttp_headers_append( list, new_cstring_buffer( 0, "3", 1 ), new_cstring_buffer( 0, "4", 1 ));
        TEST_EQ( uvhttp_headers_size(list) == 2);
        list = uvhttp_headers_append( list, new_cstring_buffer( 0, "5", 1 ), new_cstring_buffer( 0, "6", 1 ));
        TEST_EQ( uvhttp_headers_size(list) == 3);

        iter = uvhttp_headers_begin( list);
        end = uvhttp_headers_end( list);

        for ( ; iter !=end ; iter = iter->next ) {
            int field = atoi(iter->field);
            int value = atoi(iter->value);
            TEST_EQ( field == (( c + 1) * 2 -1));
            TEST_EQ( value == (( c + 1) * 2));
            free_string_buffer( iter->field);
            free_string_buffer( iter->value);
            c ++;
        }

        uvhttp_headers_free( list);
    }
    //≤‚ ‘httpserver
    {
        uvhttp_loop loop = uvhttp_loop_new();
        if ( loop) {
            uvhttp_server server = uvhttp_server_new( loop);
            if ( server) {
                if ( uvhttp_server_ip4_listen( server, "0.0.0.0", 8011) == UVHTTP_OK) {
                }
            }
            uvhttp_run( loop);
            uvhttp_server_delete( server);
            uvhttp_loop_delete( loop);
        }
    }
}