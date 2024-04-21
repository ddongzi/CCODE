#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <syslog.h>
extern void daemonize(const char *cmd);

int main(void )
{
    daemonize("login daemon"); 
    char *file_name = "/var/log/mydaemon.log";
    FILE *file = fopen(file_name, "w");
    syslog(LOG_INFO, "pid : %ld", (long)getpid());
    if (file == NULL) {
        syslog(LOG_ERR, "file  open failed.");
        exit(1);
    }
    char *name = getlogin();
    if (name == NULL) {
        syslog(LOG_ERR, "getlogin name failed.");
        exit(1);
    }
    fprintf(file, "%s\n", name);
    fclose(file);
    return 0;

}
