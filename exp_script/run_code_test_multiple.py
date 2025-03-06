import os
import time
import sys
import argparse
import sys
import subprocess
import re
from itertools import combinations

# import common configs
import common

DEFAULT_BLOCK_SIZE = 1048576

def parse_args(cmd_args):
    argParser = argparse.ArgumentParser(description="run code test") 

    # Input parameters: -f code test file
    argParser.add_argument("-f", type=str, required=True, help="code test file (.txt): each line represents a code name with parameters")
    
    # Input parameters: -f code test file
    argParser.add_argument("-n", type=str, required=True, help="number of failures")
    
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
    numFailures = int(args.n)
    codeList = getCodeList(codeTestListFile)

    startExpTime = time.time()

    print(codeList)

    # generate OpenEC configs with codeList
    print("Generate OpenEC configs")
    genOECConfigScript = os.path.join(common.EXP_SCRIPT_DIR, "gen_oec_config.py")
    cmd = "python3 {} -f {}".format(genOECConfigScript, common.EVAL_SETTING_FILE)
    execCmd(cmd)

    print("Run OpenEC CodeTest for each code")
    for code in codeList:
        codeName = code[0]
        codeN = code[1]
        codeK = code[2]
        codeW = code[3]
        codeId = "_".join([codeName, str(codeN), str(codeK), str(codeW)])

        # Repair degree stats
        repairDegStats = []

        # Repair bandwidth stats
        repairBWStats = []
        repairBWTotalStats = []
        repairTransferNodeStats = []
        AllMinRetSubPktNode = codeW
        AllMaxRetSubPktNode = 0

        # Repair access
        nonContIOStats = []
        nonContIONodeStats = []
        AllMinNonContAccessNode = codeW
        AllMaxNonContAccessNode = 0


        # enumerate all combinations of numFailures from 0 to codeN - 1
        numCombs = 0
        for index, failedIds in enumerate(combinations(range(codeN), numFailures)):
            numCombs += 1
            failedIds = " ".join([str(x) for x in failedIds])
            # print(index, failedIds)

            cmd = "source {} && cd {} && ./{} {} {} {} {} {} {}".format("~/.zshrc", common.BUILD_DIR, "CodeTest", codeName, codeN, codeK, codeW, DEFAULT_BLOCK_SIZE, failedIds)
            msg, success = execCmd(cmd, printCmd=False, printOutputs=False)

            if "error" in msg:
                print("Error: invalid outputs: {}".format((msg)))
                exit(-1)
            
            # extract the repair degree
            resultToken = "Repair degree"
            for line in msg.split('\n'):
                line = line.strip()
                if len(line) == 0:
                    continue
                if resultToken not in line:
                    continue
                match = re.search(r"{} .*: (\d+)".format(resultToken), line)
                if not match:
                    print("Error: cannot find result from line: {}".format(line))
                    # print(msg)
                    exit()
                repairDegree = match.group(1)

                repairDegStats.append(int(repairDegree))
            
            # extract the repair bandwidth
            resultToken = "Repair bandwidth"
            for line in msg.split('\n'):
                line = line.strip()
                if len(line) == 0:
                    continue
                if resultToken not in line:
                    continue
                match = re.search(r"{}: total packets read: (\d+) / (\d+), average per node: (\d*\.\d+) \(min: (\d+), max: (\d+)\).*".format(resultToken), line)
                if not match:
                    print("Error: cannot find result from line: {}".format(line))
                    # print(msg)
                    exit()
                numRetSubPkts = match.group(1)
                numAllSubPkts = match.group(2)
                avgRetSubPktPerNode = match.group(3)
                minRetSubPktNode = match.group(4)
                maxRetSubPktNode = match.group(5)

                repairBWStats.append(int(numRetSubPkts))
                repairBWTotalStats.append(int(numAllSubPkts))
                repairTransferNodeStats.append(float(avgRetSubPktPerNode))
                AllMinRetSubPktNode = min(AllMinRetSubPktNode, int(minRetSubPktNode))
                AllMaxRetSubPktNode = max(AllMaxRetSubPktNode, int(maxRetSubPktNode))

            # extract the repair access
            resultToken = "Repair access"
            for line in msg.split('\n'):
                line = line.strip()
                if len(line) == 0:
                    continue
                if resultToken not in line:
                    continue
                match = re.search(r"{}: total non-contiguous accesses: (\d+), average per node: (\d*\.\d+) \(min: (\d+), max: (\d+)\)".format(resultToken), line)
                if not match:
                    print("Error: cannot find result from line: {}".format(line))
                    # print(msg)
                    exit()
                numNonContAccess = match.group(1)
                avgNonContAccessPerNode = match.group(2)
                minNonContAccessNode = match.group(3)
                maxNonContAccessNode = match.group(4)

                nonContIOStats.append(int(numNonContAccess))
                nonContIONodeStats.append(float(avgNonContAccessPerNode))
                AllMinNonContAccessNode = min(AllMinNonContAccessNode, int(minNonContAccessNode))
                AllMaxNonContAccessNode = max(AllMaxNonContAccessNode, int(maxNonContAccessNode))

        # Repair access

        # validate if results are correct
        if sum(repairBWTotalStats) != numCombs * codeK * codeW:
            print("Error: invalid number of total subpackets: {} != {}".format(sum(repairBWTotalStats), int(numCombs * codeW * codeK)))
            exit(-1)

        print("Repair bandwidth stats:", repairBWStats)
        print("Percents of combinations with repair bandwidth reduction: {}".format(sum([1 for x in repairBWStats if x < codeK * codeW]) / numCombs))

        # print stats
        print("Code: {}, Repair degree: {} (min: {}, max: {})".format(codeId, sum(repairDegStats) / numCombs, min(repairDegStats), max(repairDegStats)))
        print("Code: {}, Repair bandwidth: {} / {} (min: {}, max: {}), normalized: {}, average transferred blocks per node: {} (min: {}, max: {})".format(codeId, sum(repairBWStats) / codeW / numCombs, sum(repairBWTotalStats) / codeW / numCombs, min(repairBWStats) / codeW, max(repairBWStats) / codeW,                                  sum(repairBWStats) / sum(repairBWTotalStats), sum(repairTransferNodeStats) / codeW / numCombs, AllMinRetSubPktNode / codeW, AllMaxRetSubPktNode / codeW))
        print("Code: {}, Repair access: {} (min: {}, max: {}), average per node: {} (min: {}, max: {})".format(codeId, sum(nonContIOStats) / numCombs, min(nonContIOStats), max(nonContIOStats), sum(nonContIONodeStats) / numCombs, AllMinNonContAccessNode, AllMaxNonContAccessNode))
        
            

    endExpTime = time.time()
    print("Test finished, used time: {} (s)".format(str(endExpTime - startExpTime)))

if __name__ == '__main__':
    main()


