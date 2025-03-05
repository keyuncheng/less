# Evaluation Scripts

* Scripts: 
    * eval_settings.ini: evaluation settings file
    * node_list.txt: node IP 
    * code_test_list.txt: erasure codes to test in (n,k,w) tuple
    * load_eval_settings.sh: load eval_settings.ini into bash variables
    * set_login_dist.sh: create user and set root privilege on each node
    * test_login_dist.sh: test ssh connection on each node
    * set_ssh_dist.sh: setup ssh password-less connection on each node
    * set_bw.sh: set up bandwidth limit on a node
    * benchamrk_bw_dist.sh: benchmark bandwidth between current node and other
      nodes
    * benchamrk_disk.sh: benchmark disk throughput and IOPS
    * run_cmd_dist.sh: run command on each node
    * run_script_dist.sh: run script on each node


install.sh: install packages