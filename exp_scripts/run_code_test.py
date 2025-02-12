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

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
ROOT_DIR = os.path.dirname(SCRIPT_DIR)
BUILD_DIR = os.path.join(ROOT_DIR, "build")
CONFIG_DIR = os.path.join(BUILD_DIR, "config")

def parse_args(cmd_args):
    argParser = argparse.ArgumentParser(description="run code test") 

    # Input parameters: -f code test file
    argParser.add_argument("-f", type=str, required=True, help="code test file (.txt): each line represents a code name with parameters")
    
    args = argParser.parse_args(cmd_args)
    return args

def execCmd(cmd, exec=True, timeout=None):
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
    print(SCRIPT_DIR)
    print(ROOT_DIR)
    print(BUILD_DIR)
    print(CONFIG_DIR)

    Path(CONFIG_DIR).mkdir(parents=True, exist_ok=True)
    
    # generate OpenEC configs with codeList

    

    endExpTime = time.time()
    print("Test finished, used time: {} (s)".format(str(endExpTime - startExpTime)))

if __name__ == '__main__':
    main()


