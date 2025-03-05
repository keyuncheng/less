#!/bin/bash
# usage: reset and restart HDFS

source "./load_eval_settings.sh"

stop-dfs.sh

# namenode
rm -rf $hdfs_dir/tmp
rm -rf $hdfs_dir/logs

# execute command
for idx in $(seq 0 $((num_nodes-1))); do
    node_ip=${node_ip_list[$idx]}
     
    ssh -n $user_name@$node_ip "rm -rf $hdfs_dir/tmp"
    ssh -n $user_name@$node_ip "rm -rf $hdfs_dir/logs"
    ssh -n $user_name@$node_ip "rm -rf $hdfs_dir/dfs"
done

hdfs namenode -format -force && start-dfs.sh
