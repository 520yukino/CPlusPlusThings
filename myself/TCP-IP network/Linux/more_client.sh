#! /bin/bash
##获取本机ip并多次运行客户端
ip=$(ifconfig | grep 'inet 192' | sed 's/^.*inet //' | sed 's/ .*//')
for ((i=0; i<50; i++)); do
    ./client.out $ip 8888
done