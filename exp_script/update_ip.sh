#!/bin/bash

source "./load_eval_settings.sh"

if [ "$#" != "1" ]; then
	echo "Usage: $0 node_ip" >&2
    exit 1
fi

node_ip=$1

conf_filename=sysSetting.xml

sed -i "s%\(<attribute><name>local.addr</name><value>\)[^<]*%\1${node_ip}%" ${config_dir}/${conf_filename}
sed -i "s%\(<property><name>oec.local.addr</name><value>\)[^<]*%\1${node_ip}%" ${hadoop_home_dir}/etc/hadoop/hdfs-site.xml
