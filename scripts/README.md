# Evaluation Scripts

* Scripts:
    * setup.sh: setup the evaluation environment
    * load_eval_settings.sh: load eval_settings.ini into bash variables
    * download_deps.sh: download dependencies on a node
    * install_deps.sh: install dependencies on a node
    * create_users_dist.exp: on each DataNode, create user and set root privilege
    * set_ssh_dist.exp: generate ssh keys on each node, and setup
      password-less connections between NameNode and DataNodes
    * compile_hadoop.sh: compile Hadoop
    * install_oec.sh: install OpenEC
    * test_login_dist.sh: test login on all nodes

* Files:
    * settings.ini: evaluation settings file
    * common.py: common functions of python scripts
    * node_list.txt: node IP list
    * code_test_list.txt: erasure codes to test