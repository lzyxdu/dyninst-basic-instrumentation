# Overview
* mutator iterates each function in the mutatee (excluding functions from other libraries and linker-added ones), instruments each call instruction. For indirect calls, insert a call to a user-defined function. For direct calls, insert a call to printf.
* mutatee is from https://github.com/TheAlgorithms/C/blob/master/audio/alaw.c
# Prerequisite
* Download and build dyninst (v10.2.1). 
* Edit AIO.sh, env_setup.sh, and Makefile.inc to set the path to dyninst build dir.
# Usage
Simply run
```
bash AIO.sh
```
or
```
source env_setup.sh
make clean
make
./mutator
./mutatee-rewritten
```
