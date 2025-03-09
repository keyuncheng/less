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

numRuns = 10
codeParams=[("RSCode, 9, 6, 1"),("RSCode, 14, 10, 1"),("RSCode, 16, 12, 1"),("LESS, 9, 6, 2"),("LESS, 9, 6, 3"),("LESS, 14, 10, 2"),("LESS, 14, 10, 3"),("LESS, 14, 10, 4"),("LESS, 16, 12, 2"),("LESS, 16, 12, 3"),("LESS, 16, 12, 4")]
evalBlockSizeBytes=[131072, 262144, 524288, 1048576, 2097152]

RAW_LIB_DIR=common.HOME_DIR + "/array-code"
RAW_LIB_BUILD_DIR=RAW_LIB_DIR + "/build"


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

def student_t_dist(samples_arr, ci=0.95):
    samples = np.array(samples_arr)
    # sample mean
    mean = samples.mean()
    # sample standard deviation
    stdev = samples.std()
    # degree of freedom = # of samples - 1
    dof = len(samples) - 1
    # t value
    t_crit = np.abs(t.ppf((1-ci) / 2, dof))
    print('t = {:.3f} when percentage = {:.3f} and degree of freedom = {:d}'.format(t_crit, (1-ci)*100/2, dof))
    # interval
    t_diff = stdev * t_crit / np.sqrt(len(samples))

    return mean, t_diff


def main():
    print("run encoding experiment")
    
    startExpTime = time.time()



    time.sleep(1)
    
    for codeParam in codeParams:
        for blockSizeBytes in evalBlockSizeBytes:
            print(blockSizeBytes, codeParam)
            codeName = codeParam[0]
            codeN = codeParam[1]
            codeK = codeParam[2]
            codeAlpha = codeParam[3]

            encodingThroughputList = []
            encodingTimeList = []
            for i in range(numRuns):
                cmd = "cd {} && ./CodeTest {} {} {} {}".format(common.RAW_LIB_BUILD_DIR, codeName, str(codeN), str(codeK), str(codeAlpha), blockSizeBytes)
                retVal, success = execCmd(cmd, exec=True)
                # extract Encoding throughput from retVal
                # format: Encoding throughput: 1464.843750 MiB/s, time:
                # 0.000004
                resultToken = "Encoding throughput: "
                for line in retVal.split("\n"):
                    line = line.strip()
                    if line.startswith(resultToken):
                        match = re.search(r"{} (\d+\.\d+) MiB/s, time: (\d+\.\d+)".format(resultToken), line)
                        if not match:
                            print("Error: cannot find result from line: {}".format(line))
                            # print(msg)
                            exit()
                        encodingThroughput = match.group(1)
                        encodingTime = match.group(2)
                        encodingThroughputList.append(float(encodingThroughput))
                        encodingTimeList.append(float(encodingTime))
                        break
            
            # calculate average throughput
            results_std_t = student_t_dist(encodingThroughputList)
            resultsAvg = results_std_t[0]
            resultsLower = results_std_t[0] - results_std_t[1]
            resultsUpper = results_std_t[0] + results_std_t[1]

            print("Results for {}: {} {} {}".format(codeParam, resultsAvg, resultsLower, resultsUpper))

            time.sleep(1)

    end_exp_time = time.time()
    print("Evaluation finished, used time: {} (s)".format(str(end_exp_time - startExpTime)))

if __name__ == '__main__':
    main()


