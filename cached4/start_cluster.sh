#!/bin/bash

# 读取配置文件并解析节点信息
read_config() {
    local config_file="$1"
    local nodes=()

    # 检查配置文件是否存在
    if [ ! -f "$config_file" ]; then
        echo "Error: Config file '$config_file' not found."
        return 1
    fi

    # 从配置文件中提取节点信息
    while IFS= read -r line; do
        local ip=$(echo "$line" | jq -r '.ip')
        local port=$(echo "$line" | jq -r '.port')
        local role=$(echo "$line" | jq -r '.role')
        nodes+=("$ip:$port:$role")
    done < <(jq -c '.nodes[]' "$config_file")

    echo "${nodes[@]}"
}

# 检查端口是否被占用，并杀掉占用该端口的所有进程
free_port_if_in_use() {
    local port="$1"
    local pids

    pids=$(lsof -t -i:$port)

    for pid in $pids; do
        if [ -n "$pid" ]; then
            echo "Port $port is in use by process $pid. Killing process..."
            kill -9 "$pid"
            sleep 1 # 等待进程被杀掉
        fi
    done
}

# 启动数据节点
start_data_nodes() {
    local nodes=("$@")
    for node in "${nodes[@]}"; do
        local ip=$(echo "$node" | cut -d':' -f1)
        local port=$(echo "$node" | cut -d':' -f2)
        local role=$(echo "$node" | cut -d':' -f3)
        
        if [ "$role" = "data" ]; then
            free_port_if_in_use "$port"
            echo "Starting data node on $ip:$port"
            ./cache_server "$port" > "logs/cache_server_$port.log" 2>&1 &

            if [ $? -eq 0 ]; then
                echo "Data node started successfully on $ip:$port"
            else
                echo "Failed to start data node on $ip:$port"
            fi
        fi
    done
}

# 启动管理节点
start_management_nodes() {
    local nodes=("$@")
    for node in "${nodes[@]}"; do
        local ip=$(echo "$node" | cut -d':' -f1)
        local port=$(echo "$node" | cut -d':' -f2)
        local role=$(echo "$node" | cut -d':' -f3)
        
        if [ "$role" = "management" ]; then
            free_port_if_in_use "$port"
            echo "Starting management node on $ip:$port"
            ./cluster_manager "$port" > "logs/cluster_manager_$port.log" 2>&1 &

            if [ $? -eq 0 ]; then
                echo "Management node started successfully on $ip:$port"
            else
                echo "Failed to start management node on $ip:$port"
            fi
        fi
    done
}

# 主函数
main() {
    local config_file="cluster.json"
    local nodes=($(read_config "$config_file"))

    if [ ${#nodes[@]} -eq 0 ]; then
        echo "No nodes found in the config file."
        exit 1
    fi

    mkdir -p logs

    start_data_nodes "${nodes[@]}"
    start_management_nodes "${nodes[@]}"
}

# 运行主函数
main
