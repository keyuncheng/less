#!/bin/bash
# usage: set up bandwidth limit on a node

source "./load_eval_settings.sh"

if [ "$#" != "2" ]; then
	echo "Usage: $0 node_ip upload_bw (Kbps)" >&2
    exit 1
fi

node_ip=$1
upload_bw_Kbps=$2

wondershaper_dir=$home_dir/wondershaper

# node_ip=$(ifconfig | grep '192.168.10' | head -1 | sed "s/ *inet [addr:]*\([^ ]*\).*/\1/")
node_dev=$(ifconfig | grep -B1 $node_ip | grep -o "^\w*")

cd $wondershaper_dir && echo $user_passwd | sudo -S ./wondershaper -a $node_dev -u $upload_bw_Kbps
cd -

echo enable bandwidth setting: $node_ip $node_dev $upload_bw_Kbps Kbps
