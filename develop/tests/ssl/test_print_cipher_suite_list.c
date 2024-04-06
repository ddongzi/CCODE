//
// Created by dong on 4/6/24.
//
#include "ssl.h"
int main(void )
{
    printf("hello\n");
    // TODO ctest过程中打印不出来，错误输出日志有
    print_cipher_suite_list("NORMAL");
    return 0;
}