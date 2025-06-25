#!/bin/bash
# update configs: ip, block size and packet size

source "./load_eval_settings.sh"

if [ "$#" != "2" ]; then
	echo "Usage: $0 block_size (Byte) packet_size (Byte)" >&2
    exit 1
fi

block_size=$1
packet_size=$2


# update OpenEC and HDFS configs locally
python3 gen_oec_config.py $INI_FILE

cp -r $proj_dir/hdfs3.3.4-integration/conf/* ${hadoop_home_dir}/etc/hadoop

# update node list
cp node_list.txt ${hadoop_home_dir}/etc/hadoop/workers
sed -i '1d' ${hadoop_home_dir}/etc/hadoop/workers # remove the NameNode

# update NameNode IP
# change to hdfs://<namenode_ip>:9000
# name node ip: the first line of node_list.txt
namenode_ip=$(head -1 node_list.txt | sed "s/ *//g")
sed -i "s%\(<property><name>fs.defaultFS</name><value>\)[^<]*%\1hdfs://${namenode_ip}:9000%" ${hadoop_home_dir}/etc/hadoop/core-site.xml
sed -i "s%\(<property><name>oec.controller.addr</name><value>\)[^<]*%\1${namenode_ip}%" ${hadoop_home_dir}/etc/hadoop/hdfs-site.xml

# block size
block_size_MiB=`expr $block_size / 1024 / 1024`
sed -i "s%\(</ecid><base>\)[^<]*%\1${block_size_MiB}%" ${conf_dir}/${conf_filename}
sed -i "s%\(<property><name>dfs.blocksize</name><value>\)[^<]*%\1${block_size}%" ${hadoop_home_dir}/etc/hadoop/hdfs-site.xml

# packet size
sed -i "s%\(<attribute><name>packet.size</name><value>\)[^<]*%\1${packet_size}%" ${conf_dir}/${conf_filename}
sed -i "s%\(<property><name>oec.pktsize</name><value>\)[^<]*%\1${packet_size}%" ${hadoop_home_dir}/etc/hadoop/hdfs-site.xml

# others
hdfs_tmp_dir=/home/${user_name}/hadoop-3.3.4
sed -i "s%\(<property><name>hadoop.tmp.dir</name><value>\)[^<]*%\1${hdfs_tmp_dir}%" ${hadoop_home_dir}/etc/hadoop/core-site.xml

# distribute the OpenEC and HDFS configs to all nodes but self
# OpenEC
bash copy_dist.sh ${conf_dir}/${conf_filename} ${conf_dir}
# HDFS
bash copy_dist.sh ${hadoop_home_dir}/etc/hadoop ${hadoop_home_dir}/etc

# Update local IPs on all nodes
bash update_ip.sh
bash run_script_dist.sh update_ip.sh