import os
import time
import sys
import argparse
import sys
import subprocess
import re

# import common configs
import common

DEFAULT_SIM_PACKET_SIZE = 1024

def parse_args(cmd_args):
    argParser = argparse.ArgumentParser(description="run code test") 

    # Input parameters: -f code test file
    argParser.add_argument("-f", type=str, required=True, help="code test file (.txt): each line represents a code name with parameters")
    
    args = argParser.parse_args(cmd_args)
    return args

def execCmd(cmd, exec=True, timeout=None, printCmd=True, printOutputs=True):
    if printCmd == True:
        print("Execute Command: {}".format(cmd))
    msg = ""
    success=False
    if exec == True:
        with subprocess.Popen(cmd, shell=True, stdout=subprocess.PIPE) as p:
            try:
                return_str, stderr = p.communicate(timeout=timeout)
                msg = return_str.decode().strip()
                success=True
            except Exception as e:
                print(e)
                p.terminate()
        if printOutputs == True:
            print(msg)
    return msg, success


def getCodeList(codeTestListFile):
    # code name
    codeList = []
    with open(codeTestListFile, 'r') as f:
        for line in f.readlines():
            if len(line.strip()) == 0:
                continue
            items = line.strip().split(' ')
            codeName = items[0]
            codeN = int(items[1])
            codeK = int(items[2])
            codeW = int(items[3])
            codeList.append((codeName, codeN, codeK, codeW))
    return codeList


def main():
    args = parse_args(sys.argv[1:])
    if not args:
        exit()

    # Input parameters: codeTestListFile
    codeTestListFile = args.f
    codeList = getCodeList(codeTestListFile)

    startExpTime = time.time()

    print(codeList)

    # generate OpenEC configs with codeList
    print("Generate OpenEC configs")
    genOECConfigScript = os.path.join(common.SCRIPT_DIR, "gen_oec_config.py")
    cmd = "python3 {} -f {}".format(genOECConfigScript, common.EVAL_SETTING_FILE)
    execCmd(cmd)

    print("Run OpenEC CodeTest for each code")
    for code in codeList:
        codeName = code[0]
        codeN = code[1]
        codeK = code[2]
        codeW = code[3]
        codeId = "_".join([codeName, str(codeN), str(codeK), str(codeW)])

        SumNumRetrievedSubPkts = 0
        SumNumTotalSubPkts = 0
        for failedNodeId in range(codeN):
            cmd = "source {} && cd {} && ./{} {} {} {} {} {} {}".format("~/.zshrc", common.BUILD_DIR, common.CODE_TEST_BIN, codeName, codeN, codeK, codeW, DEFAULT_SIM_PACKET_SIZE, failedNodeId)
            msg, success = execCmd(cmd, printCmd=False, printOutputs=False)

            if "error" in msg:
                print("Error: invalid outputs")
                exit(-1)
            
            # extract the repair bandwidth
            result_token = "packets read:"
            for line in msg.split('\n'):
                line = line.strip()
                if len(line) == 0:
                    continue
                if result_token not in line:
                    continue
                match = re.search(r"{} ([0-9]*) / ([0-9]*), (.*)".format(result_token), line)
                if not match:
                    print("Error: cannot find result from line: {}".format(line))
                    # print(msg)
                    exit()
                numRetrievedSubPkts = match.group(1)
                numAllSubPkts = match.group(2)
                # results_raw = match.group(3)

                SumNumRetrievedSubPkts += int(numRetrievedSubPkts)
                SumNumTotalSubPkts += int(numAllSubPkts)
        OverallRepairBW = SumNumRetrievedSubPkts / SumNumTotalSubPkts
        if codeN * codeW != SumNumTotalSubPkts / codeK:
            print("Error: invalid number of total subpackets: {} != {}".format(int(codeN * codeW), int(SumNumTotalSubPkts / codeK)))
            exit(-1)
        print("Code: {}, Repair bandwidth: {} ({} / {})".format(codeId, OverallRepairBW, SumNumRetrievedSubPkts / codeN / codeW, SumNumTotalSubPkts / codeN / codeW))
            

    endExpTime = time.time()
    print("Test finished, used time: {} (s)".format(str(endExpTime - startExpTime)))

if __name__ == '__main__':
    main()


