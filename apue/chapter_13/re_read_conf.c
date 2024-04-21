//
// Created by dong on 4/20/24.
//
#include "apue.h"
#include <string.h>
#include <syslog.h>
#include <pthread.h>

extern int already_running(void );

sigset_t mask;
void reread()
{}
void *thr_fn(void *arg)
{
    int err, signo;
    for (;;) {
        err = sigwait(&mask, &signo);
        if (err != 0) {
            syslog(LOG_ERR, "sigwait failed.");
            exit(1);
        }
        switch (signo) {
            case SIGHUP:
                syslog(LOG_INFO, "Re-reading configuration file");
                reread();
                break;
            case SIGTERM:
                syslog(LOG_INFO, "got SIGTERM; exiting");
                exit(0);
            default:
                syslog(LOG_INFO, "unexpected signal %d\n", signo);
        }
    }
    return 0;
}

int main(int argc, char *argv[])
{
    pthread_t tid;
    int err;
    struct sigaction sa;
    char *cmd;
    if ((cmd = strchr(argv[0], '/')) == NULL)
        cmd = argv[0];
    else
        cmd++;

    daemonize(cmd);

    if (already_running()) {
        syslog(LOG_ERR, "daemon already running.");
        exit(1);
    }

    sa.sa_handler = SIG_DFL;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    if (sigaction(SIGHUP, &sa, NULL) < 0) {
        err_quit("%s : can't restore SIGHUP default");
    }
    sigfillset(&mask);
    if ((err = pthread_sigmask(SIG_BLOCK, &mask, NULL)) != 0) {
        err_exit(err, "SIG BLOCK error");
    }
    err = pthread_create(&tid, NULL, thr_fn, 0);
    if (err != 0)
        err_exit(err, "can't create thread");

    exit(0);
}