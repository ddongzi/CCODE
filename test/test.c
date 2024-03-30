//
// Created by dong on 3/29/24.
//
#include "test.h"

void hello()
{
    printf("hello");
}

int main(void )
{
    TEST(hello, "comment default test.");
    return 0;
}