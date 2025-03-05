#!/bin/bash
# update configs: ip, block size and packet size

source "./load_eval_settings.sh"

if [ "$#" != "2" ]; then
	echo "Usage: $0 block_size (Byte) packet_size (Byte)" >&2
    exit 1
fi

block_size=$1
packet_size=$2

# update configs
bash copy_dist.sh ${conf_dir}/${conf_filename} ${conf_dir}
cp -r ${hdfs_conf_dir}/* ${hadoop_home_dir}/etc/hadoop
chmod 777 ${hadoop_home_dir}/etc/hadoop/rack_topology.sh
bash copy_dist.sh ${hadoop_home_dir}/etc/hadoop ${hadoop_home_dir}/etc

# for each node
for idx in $(seq 0 $((num_nodes-1))); do
    node_ip=${node_ip_list[$idx]}
    echo "Update configs on node $idx: $node_ip"
    bash run_script_dist.sh update_ip.sh $node_ip
    bash run_script_dist.sh update_sizes.sh $block_size $packet_size
done