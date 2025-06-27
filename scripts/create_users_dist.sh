#!/bin/bash
# usage: create user and set root privilege on each node

source "./load_eval_settings.sh"

for idx in $(seq 0 $((num_nodes-1))); do
    node_ip=${node_ip_list[$idx]}
    
    /usr/bin/expect << EOF
   
    # ssh 
    set timeout 5
    spawn ssh -t $root_user_name@$node_ip 
    expect {
        "*yes/no" { send "yes\n"; exp_continue }
        "*password" { send "$root_user_passwd\n"; exp_continue }
    }

    # adduser
    set timeout 5
    send "sudo adduser $user_name\n" 
    expect {
        "*already exists" {exit 0}
        "*password for" {send "$root_user_passwd\n"; exp_continue}
        "*New password" {send "$user_passwd\n"; exp_continue}
        "*Retype new password" {send "$user_passwd\n"; exp_continue}
        "*Full Name" {send "\n"; exp_continue}
        "*Room Number" {send "\n"; exp_continue}
        "*Work Phone" {send "\n"; exp_continue}
        "*Home Phone" {send "\n"; exp_continue}
        "*Other" {send "\n"; exp_continue}
        "*correct?" {send "Y\n"; exp_continue}
    }
    
    # add sudo privilege
    set timeout 1
    send "sudo usermod -aG sudo $user_name\n"

    # create ssh key
    send "ssh-keygen -t rsa -P \"\" -f /home/$user_name/.ssh/id_rsa\n"
    expect {
        "*overwrite" { send "y\n"; exp_continue }
        "*Enter passphrase" { send "\n"; exp_continue }
        "*Enter same passphrase again" { send "\n"; exp_continue }
    }

    set timeout 1
    send "exit\n"
    expect eof

EOF
done

