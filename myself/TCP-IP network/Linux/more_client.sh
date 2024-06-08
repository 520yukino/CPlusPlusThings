#! /bin/bash
##获取本机ip并多次运行客户端
if [ -z $1 ]; then
    ip=$(ifconfig | grep 'inet' | sed 's/^.*inet //' | sed 's/ .*//' | head -n 1)
else
    ip=$1
fi

for ((i=0; i<50; i++)); do
    ./client.out $ip 8888
done