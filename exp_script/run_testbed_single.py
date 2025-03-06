import os
import time
import sys
import json
import argparse
import sys
import subprocess
from pathlib import Path
import numpy as np
import math
import configparser
import re

# import common configs
import common


def parseArgs(cmd_args):
    argParser = argparse.ArgumentParser(description="run testbed experiment for single block failure repair") 

    # Input parameters: -f evaluation settings file
    argParser.add_argument("-f", type=str, required=True, help="evaluation settings file (.ini)")
    
    args = argParser.parse_args(cmd_args)
    return args

def execCmd(cmd, exec=True, timeout=None):
    print("Execute Command: {}".format(cmd))
    msg = ""
    success=False
    if exec == True:
        with subprocess.Popen(cmd, shell=True, stdout=subprocess.PIPE) as p:
            try:
                retStr, stderr = p.communicate(timeout=timeout)
                msg = retStr.decode().strip()
                success=True
            except Exception as e:
                print(e)
                p.terminate()
        print(msg)
    return msg, success

class MyConfigParser(configparser.ConfigParser):
    def __init__(self, defaults=None):
        configparser.ConfigParser.__init__(self, defaults=None)
    def optionxform(self, optionStr):
        return optionStr

class DictToObject:
    def __init__(self, dictionary):
        for key, value in dictionary.items():
            setattr(self, key, value)

def findBlockAndAddr(stripe_name, blockId):
    oec_block_name = "/{}_oecobj_{}".format(stripe_name, blockId)
    find_node_ip_and_block_cmd = "hdfs fsck {} -files -blocks -locations | grep Datanode".format(oec_block_name)
    
    node_ip = "undefined_ip"
    block_name = "blk_undefined"
    
    find_ip_result, success = execCmd(find_node_ip_and_block_cmd, exec=True)
    if not success:
        print("Error finding block for stripe_name: {}".format(oec_block_name))
        return node_ip, block_name

    try:
        find_ip_result.index("blk_")
    except ValueError:
        print("oec_object {} not found in HDFS!".format(oec_block_name))
        return node_ip, block_name

    block_begin = find_ip_result.index("blk_") + len("blk_")
    block_end = find_ip_result.index("len=")
    block_meta = find_ip_result[block_begin:block_end]
    block_split = block_meta.split("_")
    block_name = block_split[0]

    ip_begin = find_ip_result.index("WithStorage[") + len("WithStorage[")
    ip_end = find_ip_result.index(",DS")
    ip_origin = find_ip_result[ip_begin:ip_end]
    ip_split = ip_origin.split(":")
    node_ip = ip_split[0]
    print("found block for stripe_name: {}: {} {}".format(oec_block_name, node_ip, block_name))
    return node_ip, block_name

def deleteBlockFile(hdfs_dir, node_ip, block_name = "*"):
    print("Start to delete block: " + node_ip + ", block: " + block_name)
    delete_cmd = "ssh " + node_ip + " \"cd {}/dfs/data/current/BP-*/current/finalized/subdir0/subdir0/ && rm blk_{}\"".format(hdfs_dir, block_name)
    execCmd(delete_cmd, exec=True)

    time.sleep(2)
    print("Delete block finished: " + node_ip + ", block: " + block_name)

def checkHDFSBlocks():
    hdfs_check_blocks_cmd="hdfs fsck -list-corruptfileblocks"
    retVal, success = execCmd(hdfs_check_blocks_cmd, exec=True)
    print(retVal)


def readFileBlock(user_name, agent_ip, oec_dir, filename, num_runs):
    print("Start to read file {} for {} runs".format(filename, num_runs))

    num_success_reads = 0

    read_time_list = []
    for i in range(num_runs):
        read_cmd = "ssh {}@{} \"cd {} && ./OECClient read {} {}\"".format(user_name, agent_ip, oec_dir, "/" + filename, filename)
        retVal, success = execCmd(read_cmd, exec=True, timeout=300)

        if not success:
            print("Error: timeout reading object {} for the {}-th run".format(filename, i))
            break

        read_time = -1

        try:
            retVal.index("duration:")
        except ValueError:
            print("Error reading object {}".format(filename))
            continue

        match = re.search(r".*read.overall.duration: (\d+\.\d+|\d+)", retVal)
        if not match or not match.groups():
            print("Error matching the results {}".format(filename))
            continue
        # print(line)
        read_time = float(match.group(1))

        # begin_p = retVal.index("duration:") + len("duration:") + 1
        # end_p = retVal.index("\n")

        # if end_p - begin_p <= 0:
        #     print("Error: read file failed")
        # else:
        #     read_time = float(retVal[begin_p:end_p])

        num_success_reads += 1
        read_time_list.append(read_time)

        time.sleep(1)

    if num_success_reads == num_runs:
        return read_time_list, True
    else:
        return read_time_list, False


def main():
    args = parseArgs(sys.argv[1:])
    if not args:
        exit()

    startExpTime = time.time()

    # Input parameters: exp_settings_file
    evalSettingsFile = args.f

    # 0. Load configurations
    configsRaw = MyConfigParser()
    configsRaw.read(evalSettingsFile)

    # 0.1 parse configurations
    configs = DictToObject({section: dict(configsRaw[section]) for section in configsRaw.sections()})

    # experiments
    configs.Experiment = DictToObject(configs.Experiment)
    exp = configs.Experiment
    exp.code_test_list_file = str(exp.code_test_list_file)
    # extract codes
    exp.codes = []
    with open(exp.code_test_list_file, 'r') as f:
        for line in f.readlines():
            if len(line.strip()) == 0:
                continue
            # format: code_name ecn eck ecw
            code_params = line.strip().split(' ')
            exp.codes.append(code_params)

    exp.num_runs = int(exp.num_runs)

    # cluster settings
    configs.Cluster = DictToObject(configs.Cluster)
    cluster = configs.Cluster
    cluster.bandwidth_kbps = int(cluster.bandwidth_kbps)
    cluster.controller_id = int(cluster.controller_id)
    cluster.agent_ids = list(map(int, cluster.agent_ids.split(',')))
    # load nodes
    cluster.nodeIps=[]
    # load node_ips
    with open(cluster.node_list_file, 'r') as f:
        for line in f.readlines():
            if len(line.strip()) == 0:
                continue
            # format: node_ip
            node_ip = line.strip()
            cluster.nodeIps.append(node_ip)
    cluster.num_nodes=len(cluster.nodeIps)

    # openec configs
    configs.OpenEC = DictToObject(configs.OpenEC)
    openec = configs.OpenEC
    openec.block_size_byte = int(openec.block_size_byte)
    openec.packet_size_byte = int(openec.packet_size_byte)
    blockSizeMiB = int(openec.block_size_byte / 1024 / 1024)

    print("Configurations:")
    print(vars(exp))
    print(vars(cluster))
    print(vars(openec))

    # clear network bandwidth
    cmd = "cd {} && bash run_script_dist.sh clear_bw.sh".format(common.EXP_SCRIPT_DIR)
    execCmd(cmd, exec=False)

    # generate OpenEC configurations
    cmd = "cd {} && python3 gen_oec_config.py -f {}".format(common.EXP_SCRIPT_DIR, evalSettingsFile)
    execCmd(cmd, exec=False)

    # update configurations
    cmd = "cd {} && bash -i update_configs_dist.sh {} {}".format(common.EXP_SCRIPT_DIR, openec.block_size_byte, openec.packet_size_byte)
    execCmd(cmd, exec=False)

    # restart hdfs and openec
    cmd = "cd {} && bash restart_oec.sh".format(common.EXP_SCRIPT_DIR)
    execCmd(cmd, exec=False)

    time.sleep(1)
    
    for code_params in exp.codes:
        codeName = code_params[0]
        ecn = int(code_params[1])
        eck = int(code_params[2])
        ecw = int(code_params[3])
        codeId = "{}_{}_{}_{}".format(codeName, ecn, eck, ecw)

        print("Start evaluating code {}".format(codeId))

        # evaluation workflow: (1) write n stripes; (2) delete the i-th block
        # from the i-th stripe; (3) repair the i-th block from the i-th stripe
        # at the corresponding node; (4) repeat for all n stripes

        print("Write {} stripes".format(ecn))
        # generate input file on the first data node
        inputFileSizeMiB = blockSizeMiB * eck
        inputFileName = "{}/{}MiB".format(common.PROJ_DIR, inputFileSizeMiB)
        cmd = "ssh {}@{} \"test -f {} && echo yes || echo no\"".format(cluster.user_name, cluster.nodeIps[0], inputFileName)
        retVal, success = execCmd(cmd, exec=False)
        if "yes" in retVal:
            print("Input file exists: {}; skip generating".format(inputFileName))
        else:
            print("Input file not exists: {}; generate".format(inputFileName))
            cmd = "ssh {}@{} \"dd if=/dev/urandom of={} bs={}MiB count={} iflag=fullblock\"".format(cluster.user_name, cluster.nodeIps[0], inputFileName, blockSizeMiB, eck)
            execCmd(cmd, exec=False)

        # write data on the first data node
        for blockId in range(ecn):
            print("Write {}-th stripe for code {}".format(blockId, codeId))
            stripeName = "_".join(["Stripe", str(blockId), codeId])
            cmd = "ssh {}@{} \"cd {} && ./OECClient write {} {} {} online {}\"".format(cluster.user_name, cluster.nodeIps[0], common.PROJ_DIR, inputFileName, "/" + stripeName, codeId, inputFileSizeMiB)
            execCmd(cmd, exec=False)
            # time.sleep(1)

        # clear network bandwidth
        cmd = "cd {} && bash run_script_dist.sh clear_bw.sh".format(common.EXP_SCRIPT_DIR)
        execCmd(cmd, exec=False)

        # set network bandwidth
        cmd = "cd {} && bash run_script_dist.sh set_bw.sh {}".format(common.EXP_SCRIPT_DIR, cluster.bandwidth_kbps)
        execCmd(cmd, exec=False)

        # TBD: resume here
        for blockId in range(ecn):
            print("Start evaluating code {}, repair block {}".format(codeId, blockId))

            stripeName = "_".join(["Stripe", str(blockId), codeId])

            # find block and node ip
            nodeIp, blockFileName = findBlockAndAddr(stripeName, blockId)

            # delete node block
            deleteBlockFile(common.HADOOP_DIR, nodeIp, blockFileName)

            time.sleep(10)
            # check if the block is corrupted
            checkHDFSBlocks()

            # degraded read file
            readFileName = "{}_{}".format(stripeName, blockId)
            readTimeList, success = readFileBlock(cluster.user_name, nodeIp, common.PROJ_DIR, readFileName, exp.num_runs)

            # if not success, retry experiment for this block
            if not success:
                blockId -= 1
                continue

            # save results
            resultSaveFolder = "{}/eval_results/{}/".format(common.PROJ_DIR, codeId)
            Path(resultSaveFolder).mkdir(parents=True, exist_ok=True)
            resultSavePath = resultSaveFolder + "block_{}.json".format(blockId)
            with open(resultSavePath, 'w',encoding='utf8') as f:
                f.write(" ".join(str(item) for item in readTimeList) + "\n")
            print("result for code {} block {}: {}".format(codeId, blockId, " ".join(str(item) for item in readTimeList)))    
            print("save {} result in {}".format(codeId, resultSavePath))

            # increment blockId
            blockId += 1

            print("Finished evaluation for code {}, block {}".format(codeId, blockId))

        print("Finished evaluating code {}".format(codeId))

        # clear network bandwidth
        cmd = "cd {} && bash run_script_dist.sh clear_bw.sh".format(common.EXP_SCRIPT_DIR)
        execCmd(cmd, exec=False)

    end_exp_time = time.time()
    print("Evaluation finished, used time: {} (s)".format(str(end_exp_time - startExpTime)))

if __name__ == '__main__':
    main()


