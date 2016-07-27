#include <stdio.h>
#include "unittest.h"
#include "uvhttp.h"

void do_test01(){
    TEST_EQ(0==0);
    TEST_EQ(1==1);
    TEST_EQ(0==01);
}