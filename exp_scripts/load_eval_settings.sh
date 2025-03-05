#!/bin/bash
# usage: load eval_settings.ini into bash variables

home_dir=$(echo ~)
proj_dir=$(dirname "$(pwd)")
build_dir=${proj_dir}/build
conf_dir=${build_dir}/conf
src_dir=${proj_dir}/src
script_dir=${proj_dir}/exp_scripts
INI_FILE=${script_dir}/eval_settings.ini
node_list_file=${script_dir}/node_list.txt

# print dir
echo "proj_dir:" $proj_dir
echo "build_dir:" $build_dir
echo "conf_dir:" $conf_dir

# root user
root_user_name=$(grep '^root_user_name = ' "$INI_FILE" | cut -d '=' -f2)
root_user_passwd=$(grep '^root_user_passwd = ' "$INI_FILE" | cut -d '=' -f2)

# user
user_name=$(grep '^user_name = ' "$INI_FILE" | cut -d '=' -f2)
user_passwd=$(grep '^user_passwd = ' "$INI_FILE" | cut -d '=' -f2)

# print user
echo "root_user_name:" $root_user_name
echo "root_user_passwd:" $root_user_passwd
echo "user_name:" $user_name
echo "user_passwd:" $user_passwd

# get node list
node_ip_list=()

while IFS= read -r line
do
    ip=`echo $line | cut -d " " -f 1`
    
    node_ip_list+=($ip)
done < ${node_list_file}

num_nodes=${#node_ip_list[@]}

# print configs
echo "node_list_file:" $node_list_file
echo Number of nodes: $num_nodes

for idx in $(seq 0 $((num_nodes-1))); do
    echo Node $idx IP: ${node_ip_list[$idx]}
done
