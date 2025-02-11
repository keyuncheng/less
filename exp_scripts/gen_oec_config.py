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
import xml.etree.cElementTree as ET

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
ROOT_DIR = os.path.dirname(SCRIPT_DIR)
BUILD_DIR = os.path.join(ROOT_DIR, "build")
CONFIG_DIR = os.path.join(BUILD_DIR, "conf")

class DictToObject:
    def __init__(self, dictionary):
        for key, value in dictionary.items():
            setattr(self, key, value)

def parse_args(cmd_args):
    argParser = argparse.ArgumentParser(description="generate openec config") 

    # Input parameters: -f code test file
    argParser.add_argument("-f", type=str, required=True, help="config file (.ini): config file")
    argParser.add_argument("-o", type=str, help="output file (.xml): output file", default=CONFIG_DIR + "/sampleSysConf.xml")
    
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

def addAttributeToXMLTree(root, attrName, attrVal):
    attribute = ET.SubElement(root, "attribute")

    name = ET.SubElement(attribute, "name")
    name.text = attrName

    value = ET.SubElement(attribute, "value")
    value.text = attrVal

def addAttributeListToXMLTree(root, attrName, attrValList):
    attribute = ET.SubElement(root, "attribute")

    name = ET.SubElement(attribute, "name")
    name.text = attrName

    for item in attrValList:
        value = ET.SubElement(attribute, "value")
        value.text = item

def main():
    args = parse_args(sys.argv[1:])
    if not args:
        exit()
    
    # Input parameters: configFile
    configFile = args.f
    outputConfigXMLFile = args.o

    # parse configs
    configsRaw = configparser.ConfigParser()
    configsRaw.read(configFile)
    configs = DictToObject({section: dict(configsRaw[section]) for section in configsRaw.sections()})

    oec = DictToObject(configs.OpenEC)

    # generate xml file
    root = ET.Element("setting")
    addAttributeToXMLTree(root, "controller.addr", oec.dss_type)
    addAttributeToXMLTree(root, "oec.controller.thread.num", oec.oec_controller_thread_num)

    tree = ET.ElementTree(root)
    ET.indent(tree, space="\t", level=0)
    print("Generated config to {}".format(outputConfigXMLFile))
    tree.write(outputConfigXMLFile)



if __name__ == '__main__':
    main()