#ifndef test_util_h__
#define test_util_h__
#include <stdio.h>
#include <stdlib.h>

#define TEST_EQ( u )\
    if ( !(u) ) {\
        printf( "\033[0;31;47mFile: %s(%d),Expression:%s  Failed\033[0m\n", __FILE__, __LINE__, #u);\
        abort();\
    }else{\
        printf( "File: %s(%d),Expression:%s  \033[0;32;40mPassed\033[0m\n", __FILE__, __LINE__, #u);\
    }
#endif // test_util_h__
