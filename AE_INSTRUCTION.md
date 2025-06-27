# Artifact Evaluation Instructions

<!-- Generate table of contents -->

## Table of Contents
1. [Artifact Claims](#artifact-claims)
2. [Testbed Setup](#testbed-setup)
3. [Evaluation](#evaluation)
   - [Numerical Analysis](#numerical-analysis)
   - [Testbed Experiments](#testbed-experiments)

## Artifact Claims

Our goal is to demonstrate LESS's effectiveness in improving repair
performance as shown in our FAST 2026 paper.  For numerical analysis (i.e.,
Exp\#A1 and Exp\#A2), we expect the results to match those in our paper. For
testbed experiments (i.e., Exp\#B1 to Exp\#B5), we expect that LESS reduces
the single-block repair and full-node recovery times compared to the baseline
erasure codes. However, the testbed experiment results may vary from those in
our paper due to different factors, such as cluster sizes, machine
specifications, operating systems, and software packages.

## Hardware Dependencies

We require and assume 15 machines to form a cluster for running the
experiments with our prototype and evaluation scripts. These machines need to
be connected via a 10 Gbps network, such that they are reachable from each
other. For each machine, we recommend a quad-core CPU, 16 GiB of memory and
above, and an HDD. We use the default erasure coding parameters (n, k) = (14,
10). In particular, we use one machine to run the HDFS NameNode and the other
machines to run the HDFS DataNodes.

## Testbed Setup

**Time estimation**: ~ 10 human-minutes + ~ 120 compute minutes

**Assumptions**: To simplify the testbed setup, we assume all nodes share the
same following default configurations:

```
Operating System: Ubuntu 22.04 LTS
Path to artifact: "/home/${username}/less"
```


We provide scripts for automatic testbed setup. The scripts are tested on
Ubuntu 22.04 LTS. Note that the actual running times of the scripts depend on
the cluster sizes, machine specifications, operating systems, and software
packages.

For each node in the cluster, please follow the steps below:

Step 1: Manually set up the node with the default username/password. This
ensures automatic testbed setup with the scripts later.

Step 2: Modify ```scripts/settings.sh``` to change the evaluation settings.
There are a few settings you may need to change:

```
[Experiment]
num_runs = 1 # the number of runs in experiments

[Cluster]
user_name = cc # the username of the root user
user_passwd = # the password of the root user (For Chameleon cloud, please keep this empty.)
user_public_key = /home/cc/.ssh/id_rsa.pub # Path to cluster SSH public key
user_private_key = /home/cc/.ssh/id_rsa # Path to cluster SSH private key (Only used for Chameleon Cloud instance created with CC-* series system image; make sure this private key is used for ssh to the instance). For nodes without private_key, please keep this empty. We assume all the nodes have the same path.
ip_prefix = 192.168.10 # the IP prefix of the cluster nodes
```

Step 3: Modify ```scripts/node_list.txt``` to include the IP addresses of all
nodes in the cluster. Each line should contain only one IP address, and the
file should not contain any empty lines or comments.  See the example below:

```
192.168.10.1
192.168.10.2
...
192.168.10.15
```

We assume **the first machine** (with the first IP address) serves as the HDFS
NameNode, and the other 14 machines serve as HDFS DataNodes.

On the NameNode, run ```scripts/setup.sh``` to install all the software
dependencies, set up the environment variables on all nodes, and set up mutual
password-less SSH connection between all nodes.

```
cd scripts
bash setup.sh
```

To verify the installation is successful, please check the following items:

* Check if the terminal outputs any essential error messages.
* Test SSH password-less login: run ```bash test_login_dist.sh``` should return "success" for all nodes.
* Redis: run ```redis-cli``` should enter the Redis CLI without any error.
* Hiredis: run ```find /usr /usr/local -name hiredis.h 2>/dev/null``` should
  return the path to the hiredis header file, e.g.,
  ```/usr/local/include/hiredis/hiredis.h```.
* GF-Complete: run ```find /usr /usr/local -name gf_complete.h 2>/dev/null``` should
  return the path to the gf_complete header file, e.g.,
  ```/usr/local/include/gf_complete.h```.
* ISA-L: run ```find /usr /usr/local -name isa-l.h 2>/dev/null``` should
  return the path to the isa-l header file, e.g.,
  ```/usr/include/isa-l.h```.
* JAVA: run ```cd $JAVA_HOME``` should enters the Java home directory
  ```/usr/lib/jvm/java-8-openjdk-amd64```.
* Maven: run command ```mvn -v``` should show the Maven version.
* Hadoop: the building process could take a long while, and it should not
  report any error. After the installation, run command ```hadoop version```
  should show the Hadoop version as 3.3.4.
* OpenEC: the compilation should generate the binaries without any error.


## Evaluation

The default OpenEC configuration file is in ```conf/sysSetting.xml```.
It includes configurations for different (n, k) = (14, 10) erasure codes:

| Code | ClassName | Parameters *(n,k)* | Sub-packetization (alpha) |
| ------ | ------ | ------ | ------ |
| RS codes | RSCONV | (14,10) | 1 |
| Clay codes | Clay | (14,10) | 256 |
| HashTag | HTEC | (14,10) | 2, 3, 4 |
| Hitchhiker (Hitchhiker-XOR+ version) | HHXORPlus | (14,10) | 2 |
| Elastic Transformation (base code: RS codes) | ETRSConv | (14,10) | 2, 3, 4 |
| LESS | LESS | (14,10) | 2, 3, 4 |

### Numerical Analysis (for a quick start)

#### Exp#A1 Single-block Repair

**Time estimation**: ~ 1 human-minutes + ~ 10 compute minutes

This experiment evaluates single-block repair performance of different
erasure codes. The results includes single-block repair I/O and I/O seeks.

Run ```exp_a1.sh``` to generate the results for different codes:
```
bash exp_a1.sh
```

The repair I/O and I/O seeks will be printed on the terminal, which exactly
matches the results in Tables 2 and 3. See the results for LESS (14,10,4) below:

```
Code: LESS (14, 10, 4)
Repair I/O (bandwidth): avg: 4.642857142857143, min: 4.0, max: 4.75
I/O seeks: avg: 13.0, min: 13, max: 13
```

#### Exp#A2 Multi-block Repair

**Time estimation**: ~ 1 human-minutes + ~ 5 compute minutes

This experiment evaluates multi-block repair performance of LESS and RS codes.
The results includes two-block repair I/O, I/O seeks, and the improved repair
ratio.  Note that there is no need to run experiment for RS codes, as the
repair I/O and I/O seeks are both equal to k.

Run ```exp_a2.sh``` to generate the results for LESS:
```
bash exp_a2.sh
```

The repair I/O, I/O seeks, and improved repair ratio will be printed on
the terminal, which exactly matches the results in Figure 3. See the results
for LESS (14,10,2) below:
```
LESS(n=14, k=10, alpha=2, 2-blocks repair)
Repair I/O (bandwidth): avg: 9.252747252747254
I/O seeks: avg: 10.571428571428571
Improved repair (with reduced repair I/O and I/O seeks) ratio: 0.2857142857142857
```
LESS reduces the average repair I/O of RS codes by (10 - 9.252747252747254) /
10 = 7.4%, and improves for 28.6% of cases.


### Testbed Experiments

#### Exp#B1 Single-block Repair

**Time estimation**: ~ 1 human-minutes + ~ 10 compute minutes

This experiment evaluates single-block repair time of different erasure codes.
We measure single-block repair time (in seconds) in OpenEC.

Run ```exp_b1.sh``` to generate the results for LESS:
```
bash exp_b1.sh
```

The repair time will be printed on the terminal. See the results for LESS (14,10,4) below:

```
LESS (14, 10, 4), Single-block repair time (seconds): avg: 2.123456789, lower: 2.0, upper: 2.3
```

Note that the actual repair time depends on the hardware and software configurations of the testbed, and could be different from the paper's results.

#### Exp#B2 Full-node Recovery

**Time estimation**: ~ 1 human-minutes + ~ 10 compute minutes

This experiment evaluates full-node recovery time of different erasure codes.
We measure full-node recovery time (in seconds) in OpenEC.

Run ```exp_b2.sh``` to generate the results for LESS:
```
bash exp_b2.sh
```

The repair time will be printed on the terminal. See the results for LESS (14,10,4) below:

```
LESS (14, 10, 4), Full-node recovery time (seconds): avg: 2.123456789, lower: 2.0, upper: 2.3
```

#### Exp#B3 Encoding Throughput

**Time estimation**: ~ 1 human-minutes + ~ 10 compute minutes

This experiment evaluates encoding throughput (MiB/s) of LESS and RS codes. We
run a standalone program to encode a single (14,10) stripe in memory, with
packet sizes varied from 128 KiB t o 1024 KiB.

Run ```exp_b3.sh``` to generate the results for LESS and RS codes:
```
bash exp_b3.sh
```

The encoding throughputs will be printed on the terminal.  See the results for
LESS (14,10) with 256 KiB packet size below:

```
LESS (14, 10, 4), packet size (bytes): 262144, Encoding throughput (MiB/s): avg: 1643.149958, lower: 1611.547963, upper: 1675.171492
```

Note that the encoding throughputs depend on the CPUs and could be different
from the paper's results.

#### Exp#B4 Impact of Network Bandwidth

**Time estimation**: ~ 1 human-minutes + ~ 10 compute minutes

This experiment evaluates single-block repair time of LESS, RS codes and Clay
codes. We measure single-block repair time (in seconds) in OpenEC for
different network bandwidth (1, 2, 5, 10 Gbps).

Run ```exp_b4.sh``` to generate the results for LESS:
```
bash exp_b4.sh
```

The repair time will be printed on the terminal. See the results for LESS (14,10,4) below:

```
LESS (14, 10, 4), Single-block repair time (seconds): avg: 2.123456789, lower: 2.0, upper: 2.3
```

#### Exp#B5 Impact of Packet Size

**Time estimation**: ~ 1 human-minutes + ~ 10 compute minutes

This experiment evaluates single-block repair time of LESS, RS codes and Clay
codes. We measure single-block repair time (in seconds) in OpenEC for
different packet sizes (128 KiB, 256 KiB, 512 KiB, 1024 KiB).

Run ```exp_b5.sh``` to generate the results for LESS:
```
bash exp_b5.sh
```

The repair time will be printed on the terminal. See the results for LESS (14,10,4) below:

```
LESS (14, 10, 4), Single-block repair time (seconds): avg: 2.123456789, lower: 2.0, upper: 2.3
```
