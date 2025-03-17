# LESS

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

For details, please refer to the documentation ```README.md``` in corresponding subdirectories.