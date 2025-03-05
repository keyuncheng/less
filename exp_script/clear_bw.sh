#!/bin/bash
# usage: clear bandwidth setting on a node
source "./load_eval_settings.sh"

if [ "$#" != "1" ]; then
	echo "Usage: $0 node_ip" >&2
    exit 1
fi

node_ip=$1

wondershaper_dir=$home_dir/wondershaper

# cur_ip=$(ifconfig | grep '192.168.10' | head -1 | sed "s/ *inet [addr:]*\([^ ]*\).*/\1/")
node_dev=$(ifconfig | grep -B1 $node_ip | grep -o "^\w*")

cd $wondershaper_dir && echo $user_passwd | sudo -S ./wondershaper -c -a $node_dev

echo clear bandwidth setting [ $cur_ip $node_dev ]