#!/bin/bash
# usage: install packages locally
# NOTE: please ensure all packages are downloaded accordingly

source "./load_eval_settings.sh"

if [ -d $pkg_dir ];then
    echo "package directory $pkg_dir exist, continue"
else
    echo "error: package directory $pkg_dir not exit, abort"
    exit 0
fi


# dependencies (general)
echo $user_passwd | sudo -S apt-get -y install expect iproute2 net-tools fio lshw iperf python2.7 build-essential


# dependencies (cmake g++)
echo $user_passwd | sudo -S apt-get -y install cmake g++


# dependencies (redis v3.2.8)
cd $pkg_dir
tar -zxvf redis-3.2.8.tar.gz
cd redis-3.2.8
make
echo $user_passwd | sudo -S make install
cd utils
/usr/bin/expect <(cat << EOF
set timeout -1 
spawn sudo ./install_server.sh
expect {
    "*password" { send "$user_passwd\n"; exp_continue }
    "*redis port" { send "\n"; exp_continue }
    "*redis config file" { send "\n"; exp_continue }
    "*redis log file" { send "\n"; exp_continue }
    "*data directory" { send "\n"; exp_continue }
    "*redis executable" { send "\n"; exp_continue }
    "*to abort." { send "\n"; exp_continue }
}
EOF
)
echo $user_passwd | sudo -S service redis_6379 stop
echo $user_passwd | sudo -S sed -i 's/bind 127.0.0.*/bind 0.0.0.0/g' /etc/redis/6379.conf
echo $user_passwd | sudo -S service redis_6379 start


# dependencies (hiredis v0.13.3)
cd $pkg_dir
tar zxvf hiredis-0.13.3.tar.gz
cd hiredis-0.13.3
make
echo $user_passwd | sudo -S make install


# dependencies (gf-complete Ceph's mirror)
echo $user_passwd | sudo -S apt-get -y install unzip libtool autoconf yasm nasm
cd $pkg_dir
unzip gf-complete.zip
cd gf-complete-master/
libtoolize
autoupdate
./autogen.sh
./configure
make
echo $user_passwd | sudo -S make install


# dependencies (ISA-L v2.30.0)
cd $pkg_dir
tar zxvf isa-l-2.30.0.tar.gz
cd isa-l-2.30.0/
libtoolize
autoupdate
./autogen.sh
./configure
make
echo $user_passwd | sudo -S make install


# dependencies (Java 8, Maven)
echo $user_passwd | sudo -S apt-get -y install openjdk-8-jdk maven

sed -i '/JAVA_HOME=/d' $home_dir/.bashrc
sed -i '/MAVEN_HOME=/d' $home_dir/.bashrc
sed -i '/PATH=/d' $home_dir/.bashrc

echo -e '' >> $home_dir/.bashrc
echo -e 'export JAVA_HOME=/usr/lib/jvm/java-8-openjdk-amd64' >> $home_dir/.bashrc
echo -e 'export MAVEN_HOME=/usr/share/maven' >> $home_dir/.bashrc
echo -e 'export PATH=$MAVEN_HOME/bin:$JAVA_HOME/bin:$PATH' >> $home_dir/.bashrc


# dependencies (Hadoop 3.3.4)
echo $user_passwd | sudo -S apt-get -y install ezlib1g-dev libssl-dev doxygen protobuf-compiler libprotobuf-dev libprotoc-dev libsasl2-dev libgsasl7-dev libuuid1 libfuse-dev
cd $pkg_dir
tar zxf hadoop-3.3.4-src.tar.gz

# update environment variables
sed -i '/HADOOP_SRC_DIR=/d' $home_dir/.bashrc
sed -i '/HADOOP_HOME=/d' $home_dir/.bashrc
sed -i '/HADOOP_CLASSPATH=/d' $home_dir/.bashrc
sed -i '/CLASSPATH=/d' $home_dir/.bashrc
sed -i '/LD_LIBRARY_PATH=/d' $home_dir/.bashrc

echo -e "export HADOOP_SRC_DIR=$pkg_dir/hadoop-3.3.4-src" >> $home_dir/.bashrc
echo -e "export HADOOP_HOME=$home_dir/hadoop-3.3.4" >> $home_dir/.bashrc
echo -e 'export HADOOP_CLASSPATH=$JAVA_HOME/lib/tools.jar:$HADOOP_CLASSPATH' >> $home_dir/.bashrc
echo -e 'export CLASSPATH=$JAVA_HOME/lib:$CLASSPATH' >> $home_dir/.bashrc
echo -e 'export LD_LIBRARY_PATH=$HADOOP_HOME/lib/native:$JAVA_HOME/jre/lib/amd64/server/:/usr/local/lib:$LD_LIBRARY_PATH' >> $home_dir/.bashrc
echo -e 'export PATH=$HADOOP_HOME/bin:$HADOOP_HOME/sbin:$PATH' >> $home_dir/.bashrc
source $home_dir/.bashrc