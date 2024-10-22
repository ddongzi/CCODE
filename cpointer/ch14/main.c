#include <stdlib.h>
#include <stdio.h>
void print_ledger(int a)
{
#if defined(OPTION_LONG)
    printf("print_ledger_long");
#elif defined(OPTION_DETAILED)
    printf("print_ledger_detailed");
#else
    printf("print_ledger_default");
#endif
}

int main(int argc, char const *argv[])
{
    print_ledger(1);
    // int buf[ARRAYSIZE];
    return 0;
}
