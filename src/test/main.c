#include <stdio.h>
extern void do_test01();
extern void do_test02();
extern void do_test03();

int main(int argc, char* argv[])
{
    do_test01();
    do_test02();
    do_test03();
    return 0;
}
