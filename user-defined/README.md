# Overview
mutator iterates each function in the mutatee (excluding linker-added ones), instruments each call instruction. For indirect calls, insert a call to a user-defined function. For direct calls, insert a call to printf.
# Prerequisite
* Download and build dyninst (v10.2.1). 
* Edit AIO.sh or env_setup.sh to set the path to dyninst build dir.
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
