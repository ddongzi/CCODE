#!/bin/bash

# 要执行的命令
COMMAND="gnutls-cli -p \"NONE:+VERS-TLS1.0:+RSA:+AES-128-CBC:+SHA1:+SHA256:+MD5\" dong.com:4433"

# 无限循环
while true; do
    # 执行命令
    $COMMAND
    
    # 获取上一个命令的退出码
    EXIT_CODE=$?
    
    # 如果命令返回退出码，睡眠10秒钟后继续
    if [ $EXIT_CODE -ne 0 ]; then
        echo "Command exited with non-zero status. Sleeping for 10 seconds before retrying..."
        sleep 30
    fi
done

