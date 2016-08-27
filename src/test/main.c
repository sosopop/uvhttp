#include <stdio.h>
#if defined(_MSC_VER) || defined(__MINGW64_VERSION_MAJOR)
#include <crtdbg.h>
#endif
#include "unittest.h"
extern void do_test01();
extern void do_test02();
extern void do_test03();
extern void do_test04();

int main(int argc, char* argv[])
{
    do_test01();
    do_test02();
    do_test03();
    do_test04();
#if defined(_MSC_VER) || defined(__MINGW64_VERSION_MAJOR)
    TEST_EQ( _CrtDumpMemoryLeaks() == 0);
#endif
    return 0;
}
