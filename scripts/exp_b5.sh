#!/bin/bash
# usage: Exp#B5: Impact of packet size

echo "Exp#B5: Impact of packet size"

codeList=$(cat <<EOF
RSCONV 14 10 1
Clay 14 10 256
LESS 14 10 2
LESS 14 10 3
LESS 14 10 4
EOF
)

packetSizeByteList=(131072 262144 524288 1048576)

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

# vary packet size
for packet_size_byte in "${packetSizeByteList[@]}"; do
    # update in settings.ini
    sed -i "s/^packet_size_byte = .*/packet_size_byte = ${packet_size_byte}/" $INI_FILE

    # run single-block repair experiment
    echo "Running with packet size: ${packet_size_byte} bytes"
    python3 run_testbed_single_block.py

    echo "Finished with packet size: ${packet_size_byte} bytes"
done

# Extract the logs
echo "Extracting logs for single-block repair experiment"
for packet_size_byte in "${packetSizeByteList[@]}"; do
    # fetch the lines from code_test_list.txt
    for code in $(cat $exp_script_dir/code_test_list.txt); do
        # Extract the code name from the line
        code_name=$(echo $code | awk '{print $1}')
        echo "Extracting logs for code: $code_name, packet size ${packet_size_byte} bytes"

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