# Evaluation Scripts

* Scripts: 
    * bug_fix.sh: fix bugs on a node in NCS testbed
    * load_eval_settings.sh: load eval_settings.ini into bash variables
    * set_login_dist.sh: create user and set root privilege on each node
    * test_login_dist.sh: test ssh connection on each node
    * set_ssh_dist.sh: setup ssh password-less connection on each node
    * set_bw.sh: set up bandwidth limit on a node
    * clear_bw.sh: clear bandwidth limit on a node
    * benchmark_bw_dist.sh: benchmark bandwidth between current node and other
      nodes
    * benchmark_disk.sh: benchmark disk throughput and IOPS on a node
    * run_cmd_dist.sh: run command on each node
    * run_script_dist.sh: run script on each node
    * download_deps.sh: download dependencies on a node
    * install_deps.sh: install dependencies on a node
    * copy_dist.sh copy files (directories) to each node
    * update_conf_dist.sh: update configuration files on each node
    * update_ip.sh: update IP in configuration files on a node
    * update_sizes.sh: update block and packet sizes in configuration files on
      a node
    * update_hdfs_conf.sh: update HDFS configuration files on a node
    * compile_hdfs3.sh: compile HDFS
    * compile_oec.sh: compile OpenEC on a node
    * restart_hdfs.sh: reset and restart HDFS
    * restart_oec.sh restart OpenEC
    * gen_oec_config.py: generate OpenEC config on a node
    * run_code_test_single.py: run single failure erasure code test
    * run_code_test_multiple.py: run multiple failure erasure code test
    * run_testbed_single.py: run single failure testbed experiment
    * extract_testbed_logs.py: extract logs from testbed experiment

* Files:
    * eval_settings.ini: evaluation settings file
    * common.py: common functions of python scripts
    * node_list.txt: node IP 
    * code_test_list.txt: erasure codes to test in (n,k,w) tuple
    * code_test_list_multiple: erasure codes to test for multiple failures: (n,k,w) tuples