//
// Created by dong on 3/29/24.
//
#include <stdio.h>
#include "client.h"
int main(void )
{
#ifdef CLIENT_DEBUG
    printf("client debug on.");
#endif
    return 0;
}