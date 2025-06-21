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
erasure codes. However, the testbed experiment results may vary slightly from
those in our paper due to different factors, such as cluster sizes, machine
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

**Time estimation**: ~ 5 human-minutes + ~ 100 machine minutes

**Assumptions**: To simplify the testbed setup, we assume all nodes share the
same following default configurations:

```
Operating System: Ubuntu 22.04 LTS
OS username / password: less / less
network_interface: <please check on the machines via ifconfig>
Path to artifact: "/home/${username}/less"
```


We provide scripts for automatic testbed setup. The scripts are tested on
Ubuntu 22.04 LTS. Note that the actual running times of the scripts depend on
the cluster sizes, machine specifications, operating systems, and software
packages.

Please follow the steps below:

Step 1: manually set up the node with the default
username/password. This ensures automatic testbed setup with the scripts.

Step 2: For each node, modify ```scripts/settings.sh``` to follow the default
configurations.

Step 3: Modify ```node_list.txt``` to include the IP addresses of all nodes in
the cluster. Each line should contain only one IP address, and the file should
not contain any empty lines or comments.  See example below:

```
192.168.0.1
192.168.0.2
...
192.168.0.15
```

We assume **the first machine** serves as the HDFS NameNode, and the other 14
machines serve as HDFS DataNodes.

On the NameNode, run ```scripts/``` to install the software dependencies, set
up the environment variables on each node, and set up mutual password-less SSH
connection between all nodes.

```
bash scripts/setup.sh
```



## Evaluation

### Numerical Analysis

### Testbed Experiments