#!/bin/bash
# usage: benchmark bandwidth between current node and other nodes

source "./load_eval_settings.sh"

if [ "$#" != "1" ]; then
	echo "Usage: $0 node_ip" >&2
    exit 1
fi

node_ip=$1

iperf -s &

for idx in $(seq 0 $((num_nodes-1))); do
    node_ip=${node_ip_list[$idx]}

    echo Current Node: ${node_ip}
    ssh $user_name@$ip iperf -c ${node_ip}
done

killall iperf
