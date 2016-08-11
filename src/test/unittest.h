#ifndef TEST_UTIL_H__
#define TEST_UTIL_H__
#include <stdio.h>
#include <stdlib.h>

#define TEST_EQ( u )\
    if ( !(u) ) {\
        printf( "\033[0;31;47mFile: %s(%d),Expression:%s  Failed\033[0m\n", __FILE__, __LINE__, #u);\
        abort();\
    }else{\
        printf( "File: %s(%d),Expression:%s  \033[0;32;40mPassed\033[0m\n", __FILE__, __LINE__, #u);\
    }

void app_path( char* path, unsigned int path_size, const char* path_append);
void* run_shell( const char* cmd);
void wait_run( void* shellptr);
void del_file( const char* path);
#endif // TEST_UTIL_H__
