#ifndef SIGNAL_HANDLER_H
#define SIGNAL_HANDLER_H

#include <signal.h>

void setup_signal_handlers(void);
void handle_sigint(int sig);
void handle_sigterm(int sig);

extern volatile sig_atomic_t stop;

#endif // SIGNAL_HANDLER_H
