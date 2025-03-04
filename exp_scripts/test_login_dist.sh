#!/usr/bin/expect -f
# usage: test ssh connection on all nodes

source "./load_eval_settings.sh"

for idx in $(seq 0 $((num_nodes-1))); do
    node_ip=${node_ip_list[$idx]}
    
    expect << EOF
    
    set timeout 5
    spawn ssh $user_name@$node_ip "echo success"
    expect {
        "*yes/no" { send "yes\n"; exp_continue }
        "*password" { send "$user_passwd\n" }
    }
     
EOF
done
