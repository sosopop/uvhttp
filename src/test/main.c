#include <stdio.h>
extern void do_test01();
extern void do_test02();

int main(int argc, char* argv[])
{
    do_test01();
    do_test02();
    return 0;
}
