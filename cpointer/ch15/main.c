#include <stdio.h>
#include <stdlib.h>

#define MAX_LINE_LENGTH 1024

void copy_lines(FILE *input, FILE *output)
{
    char buffer[MAX_LINE_LENGTH];
    while (fgets(buffer, MAX_LINE_LENGTH, input) != NULL) {
        fputs(buffer, output);
    }
    
}

int main(int argc, char const *argv[])
{
    printf("%-5s\n","abc");

    // FILE *input = fopen("1.txt", "rw");
    // FILE *output = fopen("2.txt", "w");
    // if (input != NULL && output != NULL) {
    //     copy_lines(input, output);
    // } else {
    //     perror("fopen failed");
    //     exit(EXIT_FAILURE);
    // }
    // char buffer[2];
    // char* r = fgets(buffer, 2, stdin);
    // if (r == NULL) {
    //     perror("fgets buffer size is 1.");
    //     exit(EXIT_FAILURE);
    // }
    // printf("%s", buffer);

    // FILE *input;
    // input = fopen("data", "r");
    // if (input == NULL) {
    //     perror("data3");
    //     exit(EXIT_FAILURE);
    // }
    return 0;
}
