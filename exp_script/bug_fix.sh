#!/usr/bin/expect -f
# usage: install packages locally
# NOTE: please ensure all packages are downloaded accordingly

source "./load_eval_settings.sh"

expect << EOF

# write string command
set timeout -1 
spawn bash -c "echo -n -e \"\$\" | sudo tee /etc/needrestart/conf.d/no-prompt.conf"
expect {
    "*password" { send "$user_passwd\n"; exp_continue }
}

set timeout -1
spawn bash -c "echo -e \"nrconf{restart} = \'a\';\" | sudo tee -a /etc/needrestart/conf.d/no-prompt.conf"
expect {
    "*password" { send "$user_passwd\n"; exp_continue }
}

set timeout -1 
spawn bash -c "echo -n -e \"\$\" | sudo tee -a /etc/needrestart/conf.d/no-prompt.conf"
expect {
    "*password" { send "$user_passwd\n"; exp_continue }
}

set timeout -1
spawn bash -c "echo -e \"nrconf{kernelhints} = -1;\" | sudo tee -a /etc/needrestart/conf.d/no-prompt.conf"
expect {
    "*password" { send "$user_passwd\n"; exp_continue }
}

set timeout -1 
spawn bash -c "echo -n -e \"\$\" | sudo tee -a /etc/needrestart/conf.d/no-prompt.conf"
expect {
    "*password" { send "$user_passwd\n"; exp_continue }
}

set timeout -1
spawn bash -c "echo -e \"nrconf{ucodehints} = 0;\" | sudo tee -a /etc/needrestart/conf.d/no-prompt.conf"
expect {
    "*password" { send "$user_passwd\n"; exp_continue }
}

EOF
