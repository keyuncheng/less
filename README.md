# LESS

## Overview

LESS is a family of erasure codes designed for I/O-efficient repairs by
reducing both repair I/O and I/O seeks and balancing the reductions across
all blocks.  This repository contains the source code of LESS as described in
our paper.  It contains the following components:

* **Erasure coding library** (```/ec-library```): A standalone erasure coding
  library for testing encoding and decoding computation performance.
* **Finding Primitive element** (```/less-element-search```): The brute-force search implementation of finding the primitive element for LESS
coding coefficients.
* **OpenEC patch** (```/openec-patch```): We implement LESS atop [HDFS 3.3.4](https://hadoop.apache.org/docs/r3.3.4/)
  with [OpenEC](https://www.usenix.org/conference/fast19/presentation/li)
  (USENIX FAST 2019). The folder contains the patch to OpenEC to support LESS. 

For details, please refer to the documentation ```README.md``` in
corresponding subdirectories.

## Getting Started Instructions

## Detailed Instructions


README instructions: Your artifact package must include a clearly written "README" file that describes your artifact and provides a road map for evaluation. The README must consist of two sections. A "Getting Started Instructions" section should help reviewers check the basic functionality of the artifact within a short time frame (e.g., within 30 minutes). Such instructions could, for example, be on how to build a system and apply it to a "Hello world"-sized example. The purpose of this section is to allow reviewers to detect obvious problems during the kick-the-tires phase (e.g., a broken virtual machine image). A "Detailed Instructions" section should provide suitable instructions and documentation to evaluate the artifact fully.

