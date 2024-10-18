#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define PRINT_ARR_UINT(arr, length) do { for (size_t i = 0; i < length; i++) { printf("%u\n", arr[i]); } } while (0)

float average(int n_values, ...) 
{
    va_list list;
    va_start(list, n_values);
    int sum = 0;
    for (int i = 0; i < n_values; i++)
    {
        sum += va_arg(list, int);
    }
    va_end(list);
    return sum / n_values;
}

// "123" -> 123
int ascii_to_integer(char * string)
{
    int res = 0;
    int t;
    for (int i = 0; i < strlen(string); i++) {
        t = string[i] - 48;
        res = res * 10 + t;
    }
    return res;
}
// max list
int max_list(int max_n_values, ...) 
{
    va_list list;
    va_start(list, max_n_values);
    int max = va_arg(list, int);
    for (int i = 0, n =0; i < max_n_values - 1; i++)
    {
        n = va_arg(list, int);
        max = n > max ? n : max;
        if (n == -1) {
            break;
        }
    }
    return max;
}

// my printf: %d%s%f
void my_printf(char * format, ...)
{
    va_list list;
    va_start(list, format);
    char string[128];

    char *delim = "%";
    char *token;

    for (token = strtok(format, delim); token != NULL; token = strtok(NULL, delim)) {
        /* code */
        if (*token == 'd') {
            int d = va_arg(list, int);
            for (int  r = d % 10 ; d > 0 ; d /= 10, r = d % 10) {
                putchar(r);
            }
            
        } else if (*token == 's') {
            char *s = va_arg(list, char*);
            for (int i = 0; i < strlen(s); i++)
            {
                putchar(s[i]);
            }
            
        } else {
            putchar('?');
        }
    }
    putchar('\n');
    
}

// print tokens
void print_tokens(char *line)
{
    char *delim = "%";
    char *token = strtok(line, delim);
    printf("%s\n", token);
}


int main(int argc, char const *argv[])
{
    int arrayinfo[7] = {3, 4, 6, 1, 5, -3, 3};
    array_offset(arrayinfo, 6, 3, 1);
    // int mat1[3][2] = {
    //     {2, -6},
    //     {3,5},
    //     {1,-1}
    // };
    // int mat2[2][4] = {
    //     {4, -2, -4, -5},
    //     {-7, -3, 6, 7}
    // };
    // int *r = malloc(sizeof(int) * 12);
    // matrix_multiply(&mat1[0][0], &mat2[0][0], r, 3, 2, 4);

    // unsigned int amount;
    // char buffer[100];
    // scanf("%u", &amount);
    // written_amount(amount, buffer);
    // printf("%s\n", buffer);

    // printf("%p", buffer);
    // char s[] = "%h%d";
    // print_tokens(s);
    // char format[] = "%d%s";
    // my_printf(format, 123, "hell");

    // /* code */
    // printf("ascii_to_integer, %d\n", ascii_to_integer("218328328"));
    // printf("max_list %d\n" ,max_list(10,1,2,3,4,1,9,10,11,-1));
    return 0;
}
