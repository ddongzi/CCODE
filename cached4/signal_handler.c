#include "signal_handler.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h> // 确保包含了信号处理器头文件

volatile sig_atomic_t stop = 0;

void handle_sigint(int sig) {
    stop = 1;
    printf("Received SIGINT, stopping server...\n");
}

void handle_sigterm(int sig) {
    stop = 1;
    printf("Received SIGTERM, stopping server...\n");
}

void setup_signal_handlers(void) {
    struct sigaction sa_int, sa_term;

    sa_int.sa_handler = handle_sigint;
    sa_term.sa_handler = handle_sigterm;

    sigemptyset(&sa_int.sa_mask);
    sigemptyset(&sa_term.sa_mask);

    sa_int.sa_flags = 0;
    sa_term.sa_flags = 0;

    if (sigaction(SIGINT, &sa_int, NULL) == -1) {
        perror("Error setting up SIGINT handler");
        exit(EXIT_FAILURE);
    }

    if (sigaction(SIGTERM, &sa_term, NULL) == -1) {
        perror("Error setting up SIGTERM handler");
        exit(EXIT_FAILURE);
    }
}
