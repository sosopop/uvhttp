#include <stdio.h>
#if defined(_MSC_VER) || defined(__MINGW64_VERSION_MAJOR)
#include <crtdbg.h>
#endif
#include "unittest.h"
extern void do_test01();
extern void do_test02();
extern void do_test03();
extern void do_test04();
extern void do_test05();
extern void do_test06();
extern void do_test07();
extern void do_test08();

int main(int argc, char* argv[])
{
    do_test01();
    do_test02();
    do_test03();
    do_test04();
    do_test05();
    do_test06();
    do_test07();
    do_test08();
#if defined(_MSC_VER) || defined(__MINGW64_VERSION_MAJOR)
    TEST_EQ( _CrtDumpMemoryLeaks() == 0);
#endif
    return 0;
}
