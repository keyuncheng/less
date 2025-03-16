# LESS Find Primitive Element


## Overview

This repository contains the brute-force search implementation for finding the
primitive element for (n,k,\alpha) LESS.  The coding operations is based on
[Jerasure](https://github.com/ceph/jerasure).

## Installation

The library can be installed in Ubuntu 22.04.


* Install dependencies

```
apt-get install libtool autoconf
```

* Compile the library

```
mkdir build && cd build
cmake .. && make
```

## Run

Run the following command to find the primitive element for (n,k,alpha) LESS.

Example 1: Find primitive element for (n=14,k=10,alpha=2) under GF(2^8)

```
./LESS_search 14 10 2 8

---
The primitive element 6 in GF(2^8) satisfy the MDS property!!!
```
