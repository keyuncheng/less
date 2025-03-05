#!/bin/bash
# usage: run command on all nodes

source "./load_eval_settings.sh"

# execute command
for idx in $(seq 0 $((num_nodes-1))); do
    node_ip=${node_ip_list[$idx]}
    
    # install expect
    echo ssh -n $user_name@$node_ip "echo $user_passwd | sudo -S apt-get -y install expect"
    ssh -n $user_name@$node_ip "echo $user_passwd | sudo -S apt-get -y install expect"
    
    # ssh -n $user_name@$node_ip "rm -rf /home/kycheng/hadoop-3.3.4/etc/hadoop/hadoop"
    # ssh -n $user_name@$node_ip "echo $HADOOP_HOME"
done
