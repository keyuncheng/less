#!/bin/bash
# usage: set up bandwidth limit on a node

source "./load_eval_settings.sh"

if [ "$#" != "1" ]; then
	echo "Usage: $0 upload_bw (Kbps)" >&2
    exit 1
fi

upload_bw_Kbps=$1

wondershaper_dir=$home_dir/wondershaper

node_ip=$(ifconfig | grep $ip_prefix | head -1 | sed "s/ *inet [addr:]*\([^ ]*\).*/\1/")
node_dev=$(ifconfig | grep -B1 $node_ip | grep -o "^\w*")

cd $wondershaper_dir && echo $user_passwd | sudo -S ./wondershaper -a $node_dev -u $upload_bw_Kbps
cd -

echo enable bandwidth setting: $node_ip $node_dev $upload_bw_Kbps Kbps
