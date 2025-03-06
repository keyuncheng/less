#!/bin/bash
# update sizes: block size and packet size

source "./load_eval_settings.sh"

hdfs_tmp_dir=/home/${user_name}/hadoop-3.3.4
sed -i "s%\(<property><name>hadoop.tmp.dir</name><value>\)[^<]*%\1${hdfs_tmp_dir}%" ${hadoop_home_dir}/etc/hadoop/core-site.xml

