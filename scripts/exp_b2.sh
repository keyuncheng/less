#!/bin/bash
# usage: Exp#B2: Full-node recovery

set -e

source "./load_eval_settings.sh"

echo "Exp#B2: Full-node recovery"

# install dependencies
echo $user_passwd | sudo -S apt-get install python3-pip
pip3 install pathlib numpy scipy

codeList=$(cat <<EOF
RSCONV 14 10 1
Clay 14 10 256
HHXORPlus 14 10 2
HTEC 14 10 2
HTEC 14 10 3
HTEC 14 10 4
ETRSConv 14 10 2
ETRSConv 14 10 3
ETRSConv 14 10 4
LESS 14 10 2
LESS 14 10 3
LESS 14 10 4
EOF
)

# prepare code list
echo "${codeList[@]}" > $exp_script_dir/code_test_list.txt

# default bandwidth
bandwidth_kbps=1048576
# update in settings.ini
sed -i "s/^bandwidth_kbps = .*/bandwidth_kbps = ${bandwidth_kbps}/" $INI_FILE

# default block size
block_size_byte=67108864
# update in settings.ini
sed -i "s/^block_size_byte = .*/block_size_byte = ${block_size_byte}/" $INI_FILE

# default packet size
packet_size_byte=262144
# update in settings.ini
sed -i "s/^packet_size_byte = .*/packet_size_byte = ${packet_size_byte}/" $INI_FILE

# run full-node recovery experiment
python3 run_testbed_full_node.py

# Extract the logs
echo "Extracting logs for full-node recovery experiment"
# fetch the lines from code_test_list.txt
for code in $(cat $exp_script_dir/code_test_list.txt); do
    # Extract the code name from the line
    code_name=$(echo $code | awk '{print $1}')
    echo "Extracting logs for code: $code_name"

    ecn=$(echo $code | awk '{print $2}')
    eck=$(echo $code | awk '{print $3}')
    ecw=$(echo $code | awk '{print $4}')
    code_id="${code_name}_${ecn}_${eck}_${ecw}"

    bandwidth_Gbps=$((bandwidth_kbps / 1024 / 1024))
    block_size_MiB=$((block_size_byte / 1024 / 1024))
    packet_size_KiB=$((packet_size_byte / 1024))
    
    # Run the log extraction script
    python3 extract_testbed_logs_full_node.py -r $numRuns -d $proj_dir/eval_results/full-node/${code_id}/bw${bandwidth_Gbps}Gbps_blk${block_size_MiB}MiB_pkt${packet_size_KiB}KiB

    echo ""
done
