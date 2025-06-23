#!/bin/bash
# usage: download dependencies

source "./load_eval_settings.sh"

mkdir -p $pkg_dir && cd $pkg_dir

# redis v3.2.8
wget https://download.redis.io/releases/redis-3.2.8.tar.gz

# hiredis v0.13.3
wget https://github.com/redis/hiredis/archive/refs/tags/v0.13.3.tar.gz -O hiredis-0.13.3.tar.gz

# gf-complete Ceph's mirror
wget https://github.com/ceph/gf-complete/archive/refs/heads/master.zip -O gf-complete.zip

# ISA-L v2.30.0
wget https://github.com/intel/isa-l/archive/refs/tags/v2.30.0.tar.gz -O isa-l-2.30.0.tar.gz

# Hadoop 3.3.4
wget https://archive.apache.org/dist/hadoop/common/hadoop-3.3.4/hadoop-3.3.4-src.tar.gz

# OpenEC v1.0.0
cd $pkg_dir
git clone https://github.com/ukulililixl/openec.git

# benchmark (wondershaper)
cd $pkg_dir
git clone https://github.com/magnific0/wondershaper.git
