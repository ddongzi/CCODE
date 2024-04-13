//
// Created by dong on 3/29/24.
//

#ifndef NETWORK_CONFIG_H
#define NETWORK_CONFIG_H

#define MAX_BUFFER_SIZE 1024

#define SSL_PORT 4433

#define LOCAL_IP_ADDR "127.0.0.1"

#define HOST "dong.com"

//负责初始化和配置网络环境。
void network_config_init();
void network_config_cleanup();
#endif //NETWORK_CONFIG_H
