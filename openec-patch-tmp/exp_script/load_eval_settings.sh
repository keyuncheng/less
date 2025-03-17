#!/bin/bash
# usage: load eval_settings.ini into bash variables

home_dir=$(echo ~)
pkg_dir=${home_dir}/packages
proj_dir=$(dirname "$(pwd)")
conf_dir=${proj_dir}/conf
conf_filename=sysSetting.xml
src_dir=${proj_dir}/src
oec_script_dir=${proj_dir}/script
exp_script_dir=${proj_dir}/exp_script
hadoop_home_dir=$(echo $HADOOP_HOME)
hdfs_config_dir=${proj_dir}/hdfs3.3.4-integration/conf
INI_FILE=${exp_script_dir}/eval_settings.ini
node_list_file=${exp_script_dir}/node_list.txt

# load source 
source $home_dir/.bashrc

# print dir
echo "proj_dir:" $proj_dir
echo "conf_dir:" $conf_dir

# root user
root_user_name=$(grep '^root_user_name = ' "$INI_FILE" | cut -d '=' -f2)
root_user_name=$(echo "$root_user_name" | xargs)
root_user_passwd=$(grep '^root_user_passwd = ' "$INI_FILE" | cut -d '=' -f2)
root_user_passwd=$(echo "$root_user_passwd" | xargs)

# user
user_name=$(grep '^user_name = ' "$INI_FILE" | cut -d '=' -f2)
user_name=$(echo "$user_name" | xargs)
user_passwd=$(grep '^user_passwd = ' "$INI_FILE" | cut -d '=' -f2)
user_passwd=$(echo "$user_passwd" | xargs)

# print user
echo "root_user_name:" $root_user_name
echo "root_user_passwd:" $root_user_passwd
echo "user_name: "$user_name
echo "user_passwd:" $user_passwd

# ip_prefix
ip_prefix=$(grep '^ip_prefix = ' "$INI_FILE" | cut -d '=' -f2)
ip_prefix=$(echo "$ip_prefix" | xargs)

# print ip_prefix
echo "ip_prefix:" $ip_prefix

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
