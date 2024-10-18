#include <stdio.h>

extern int *a;
extern int b[];
int x,y;

int main(int argc, char const *argv[])
{
    printf("a: %p\nb: %p", a, b);
    x = a[3];
    y = b[3];
    return 0;
}
