#!/bin/bash
# usage: Exp#B4: Impact of network bandwidth

set -e

source "./load_eval_settings.sh"

echo "Exp#B4: Impact of network bandwidth"

# install dependencies
echo $user_passwd | sudo -S apt-get install python3-pip
pip3 install pathlib numpy scipy

codeList=$(cat <<EOF
RSCONV 14 10 1
Clay 14 10 256
LESS 14 10 2
LESS 14 10 3
LESS 14 10 4
EOF
)

bandwidthList=(1048576 2097152 5242880 10485760)

# prepare code list
echo "${codeList[@]}" > $exp_script_dir/code_test_list.txt

# default block size
block_size_byte=67108864
# update in settings.ini
sed -i "s/^block_size_byte = .*/block_size_byte = ${block_size_byte}/" $INI_FILE

# default packet size
packet_size_byte=262144
# update in settings.ini
sed -i "s/^packet_size_byte = .*/packet_size_byte = ${packet_size_byte}/" $INI_FILE

# vary network bandwidth
for bandwidth_kbps in "${bandwidthList[@]}"; do
    # update in settings.ini
    sed -i "s/^bandwidth_kbps = .*/bandwidth_kbps = ${bandwidth_kbps}/" $INI_FILE

    # run single-block repair experiment
    echo "Running with bandwidth: ${bandwidth_kbps} kbps"
    python3 run_testbed_single_block.py

    echo "Finished with bandwidth: ${bandwidth_kbps} kbps"
done

# Extract the logs
echo "Extracting logs for single-block repair experiment"
for bandwidth_kbps in "${bandwidthList[@]}"; do
    # fetch the lines from code_test_list.txt
    for code in $(cat $exp_script_dir/code_test_list.txt); do
        # Extract the code name from the line
        code_name=$(echo $code | awk '{print $1}')
        echo "Extracting logs for code: $code_name, $bandwidth_kbps kbps"

        ecn=$(echo $code | awk '{print $2}')
        eck=$(echo $code | awk '{print $3}')
        ecw=$(echo $code | awk '{print $4}')
        code_id="${code_name}_${ecn}_${eck}_${ecw}"

        bandwidth_Gbps=$((bandwidth_kbps / 1024 / 1024))
        block_size_MiB=$((block_size_byte / 1024 / 1024))
        packet_size_KiB=$((packet_size_byte / 1024))
        
        # Run the log extraction script
        python3 extract_testbed_logs_single_block.py -ecn $ecn -r $numRuns -d $proj_dir/eval_results/single/${code_id}/bw${bandwidth_Gbps}Gbps_blk${block_size_MiB}MiB_pkt${packet_size_KiB}KiB

        echo ""
    done
done
