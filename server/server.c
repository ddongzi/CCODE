#include "socket.h"
#include "connect_manager.h"
#include "message_manager.h"
#include "threadpool.h"

void server_init()
{
    socket_init();
    threadpool_init();

    message_manager_init();
    connect_manager_init(); 

}
void server_run()
{
    threadpool_run();
    connect_manager_run();
}


int main(int argc, char const *argv[])
{
    server_init();
    server_run();
    return 0;
}
