#!/usr/bin/expect -f
# usage: compile OpenEC on each node

source "./load_eval_settings.sh"

for idx in $(seq 0 $((num_nodes-1))); do
    node_ip=${node_ip_list[$idx]}
    
    expect << EOF
   
    # ssh 
    set timeout 5
    spawn ssh -t $user_name@$node_ip 
    expect {
        "*yes/no" { send "yes\n"; exp_continue }
        "*password" { send "$user_passwd\n"; exp_continue }
    }

    # compile OEC
    set timeout -1
    send "cd $proj_dir && rm -rf CMakeCache.txt && rm -rf CMakeFiles/ && cmake . -DFS_TYPE:STRING=HDFS3 && make\n"
    expect eof

    set timeout 1
    send "exit\n"
    expect eof

    echo "finished compiling OpenEC on node $idx"
EOF

done

