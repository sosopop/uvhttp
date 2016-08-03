#include <stdio.h>
#include "unittest.h"
#include "uvhttp.h"
#include "http_parser.h"

//static int http_parser_on_srv_message_begin(
//    http_parser* parser
//    )
//{
//    return 0;
//}
//
//static int http_parser_on_srv_url(
//    http_parser* parser,
//    const char *at,
//    size_t length
//    )
//{
//    return 0;
//}
//
//static int http_parser_on_srv_header_field(
//    http_parser* parser,
//    const char *at,
//    size_t length
//    )
//{
//    return 0;
//}
//
//static int http_parser_on_srv_header_value(
//    http_parser* parser,
//    const char *at,
//    size_t length
//    )
//{
//    return 0;
//}
//
//static int http_parser_on_srv_headers_complete(
//    http_parser* parser
//    )
//{
//    return 0;
//}
//
//static int http_parser_on_srv_body(
//    http_parser* parser,
//    const char *at,
//    size_t length
//    )
//{
//    return 0;
//}
//
//static int http_parser_on_srv_message_complete(
//    http_parser* parser
//    )
//{
//    return 0;
//}
//
//static int http_parser_on_srv_chunk_header(
//    http_parser* parser
//    )
//{
//    return 0;
//}
//
//static int http_parser_on_srv_chunk_complete(
//    http_parser* parser
//    )
//{
//    return 0;
//}
//static http_parser_settings srv_parser_settings = {
//    http_parser_on_srv_message_begin,
//    http_parser_on_srv_url,
//    0,
//    http_parser_on_srv_header_field,
//    http_parser_on_srv_header_value,
//    http_parser_on_srv_headers_complete,
//    http_parser_on_srv_body,
//    http_parser_on_srv_message_complete,
//    http_parser_on_srv_chunk_header,
//    http_parser_on_srv_chunk_complete
//};

void do_test01(){
    TEST_EQ(0==0);
    TEST_EQ(1==1);
    //²âÊÔuvhttp_list
    {
        struct uvhttp_list* list = 0;
        struct uvhttp_list* end = 0;
        struct uvhttp_list* iter = 0;
        int test_count = 0;

        list = uvhttp_list_append( list , (void*)4);
        TEST_EQ( uvhttp_list_size(list) == 1);
        list = uvhttp_list_append( list , (void*)5);
        TEST_EQ( uvhttp_list_size(list) == 2);
        list = uvhttp_list_append( list , (void*)6);
        TEST_EQ( uvhttp_list_size(list) == 3);

        iter = uvhttp_list_begin( list);
        end = uvhttp_list_end( list);

        for ( ; iter !=end ; iter = iter->next ) {
            test_count += (int)iter->data;
        }

        TEST_EQ( test_count == 15);

        uvhttp_list_free( list);
    }
}