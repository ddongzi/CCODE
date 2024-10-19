#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "../base.h"

void mystery(int n )
{
    n += 5;
    n /= 10;
    printf("%s\n", "*******" + 10 - n);
}
struct Point {
    int x;
    int y;
};


// 测试函数转换表
int (*test_func[])(int ) = {
    iscntrl,
    isspace
};
#define N_CATEGORY (sizeof(test_func) / sizeof(test_func[0]))
int counts[N_CATEGORY];


// int cmpare
// 元素指针
int int_compare(void * ap, void * bp)
{
    int a = *(int *)ap;
    int b = *(int *)bp;
    return a > b;
}
// 字符串比较
int str_compare(void *ap, void *bp)
{
    char *s1 = *(char* *)ap;
    char *s2 = *(char* *)bp;
    if (s1 == NULL && s2 == NULL) {
        return 0;
    }
    if (s1 == NULL) {
        return 0;
    }
    if (s2 == NULL) {
        return 1;
    }
    return *s1 > *s2;
}

// sort函数
void sort(void *arr, size_t size, size_t elem_size, int (*cmp_func)(void *, void *))
{
    for (int i = 0; i < size - 1; i++) {
        for (int j = i + 1; j < size; j++) {
            void *ap = arr + i * elem_size;
            void *bp = arr + j * elem_size;

            if (cmp_func(ap, bp)) {
                void *temp = malloc(elem_size);
                memcpy(temp,ap, elem_size);
                memcpy(ap, bp, elem_size);
                memcpy(bp, temp, elem_size);
                free(temp);
            }
        }
    }
}

// 通用命令行参数处理。
char **do_args(int argc, char **argv, char *control, 
    void (*do_arg)(int ch, char *value), 
    void (*illegal_arg)(int ch)
    )
{

}

int main(int argc, char const *argv[])
{
    char *arr[] = {
        "hello",
        "int",
        "go",
        "love",
        "you",
        NULL
    };
    size_t size = sizeof(arr) / sizeof(arr[0]) - 1;
    sort(arr, size, sizeof(arr[0]), str_compare);
    char** arr_p = arr;
    while (*arr_p != NULL) {
        printf("%s\n", *arr_p);
        arr_p++;
    }
    

    // int arr[] = {2, 3, 5, 1};
    // size_t size = sizeof(arr) / sizeof(arr[0]);
    // sort(arr, size, sizeof(int), int_compare);
    // PRINT_ARR_UINT(arr, size);

    // char s[1024];
    // scanf("%s", s);
    // char *sp = s;
    // while (*sp++ != '\0') {
    //     for (size_t i = 0; i < N_CATEGORY; i++) {
    //         counts[i] += test_func[i](*sp);            
    //     }
    // }

    // PRINT_ARR_UINT(counts, N_CATEGORY);
    

    // char *pathname = "/usr/temp/xxxxxxxxxxxxx";
    // strcpy(pathname + 10, "abcde");
    // printf("%s\n", pathname);

    // char pathname[] = "/usr/temp/";
    // strcat(pathname, "abce");
    // printf("%s\n", pathname);
    

    // struct Point p = {1, 0}, *pp;
    // pp = &p;

    // while (*++argv != NULL)
    //     printf("%s\n", *argv);    
    
    // mystery(100);
    // putchar("012345"[0]);
    return 0;
}

