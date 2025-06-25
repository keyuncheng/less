# List of Evaluation Scripts

| Name | Description |
| ------ | ------ |
| setup.sh | Set up the testbed for evaluation |
| load_eval_settings.sh | Load evaluation settings |
| download_deps.sh | Download dependencies |
| install_deps.sh | Install dependencies |
| create_users_dist.exp | (All nodes) Create user, set root privilege, and set SSH keys |
| set_ssh_dist.exp | (All nodes) Set up password-less connections |
| test_login_dist.sh | (All nodes) Test user login |
| compile_hadoop.sh | Compile Hadoop |
| install_oec.sh | Install OpenEC |
| settings.ini | Evaluation settings file |
| common.py | Common functions for Python |
| node_list.txt | List of Node IPs |
| code_test_list.txt | List of Erasure codes to test |
| set_bw.sh | Set bandwidth for the testbed |
| clear_bw.sh | Clear bandwidth settings on the testbed |
| benchmark_disk.sh | Benchmark disk performance |
| benchmark_bw_dist.sh | (All nodes) Benchmark bandwidth in the cluster |
| copy_dist.sh | (All nodes but self) Copy files to all nodes but itself |
| run_script_dist.sh | (All nodes but self) Run a script on all nodes but itself |
| update_config_dist.sh | (All nodes) Update OpenEC and HDFS configurations on all nodes
| update_hdfs_config.sh | Update HDFS configurations |
| update_ip.sh | Update IP addresses in the configuration files |
| gen_oec_config.py | Generate OpenEC configuration file |
| restart_hdfs.sh | Restart HDFS |
| restart_oec.sh | Restart OpenEC |