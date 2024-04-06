//
// Created by dong on 3/29/24.
//

#ifndef TEST_TEST_H
#define TEST_TEST_H

#include <stdio.h>
#include <stdlib.h>

#define TEST(name, comment) do { \
    printf("Running my_test: %s (%s)\n", #name, comment); \
    name();             \
    printf("Passed my_test: %s (%s)\n", #name, comment); \
} while (0)

#define ASSERT(condition, expect, res) do { \
    if (!(condition)) {          \
        fprintf(stderr, "Assertion failed : %s, line %d\n", #condition, __LINE__); \
        fprintf(stderr, "Expected : %d.\n", expect);                                           \
        fprintf(stderr, "Result : %d.\n", res);                                    \
        exit(EXIT_FAILURE);    \
    }                             \
} while(0)
#endif //TEST_TEST_H
